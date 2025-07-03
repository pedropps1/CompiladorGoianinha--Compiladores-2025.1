#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "semantico.h"
#include "tabela_simbolos.h"

/* ========================================================= */
/*  Utilidades                                               */
/* ========================================================= */

static void erro_semantico(int linha, const char *fmt, ...)
{
    fprintf(stderr, "ERRO: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, " linha %d\n", linha);
    exit(1);
}

/* Converte nó AST_INT / AST_CAR em Tipo (enum da tabela) */
static Tipo tipo_do_no(AST *noTipo)
{
    if (!noTipo)
        return TIPO_INT; /* default impossível */
    if (noTipo->tipo == AST_INT)
        return TIPO_INT;
    if (noTipo->tipo == AST_CAR)
        return TIPO_CAR;
    return TIPO_INT;
}

/* ========================================================= */
/*  Variáveis de contexto                                    */
/* ========================================================= */

static Tipo tipoFuncaoAtual = TIPO_INT; /* tipo de retorno da função em análise */

/* ========================================================= */
/*  Declarações adiantadas                                   */
/* ========================================================= */

static Tipo verifica_expr(AST *expr);
static void verifica_comando(AST *cmd);
static void percorre_bloco(AST *bloco);

/* ========================================================= */
/*  Verificação de expressões                               */
/* ========================================================= */

static Tipo verifica_operador_binario(AST *no, Tipo esperado)
{
    Tipo esq = verifica_expr(no->filhos[0]);
    Tipo dir = verifica_expr(no->filhos[1]);

    if (esq != dir)
        erro_semantico(no->linha, "tipos incompatíveis no operador %s", no->valor);

    if (esperado != esq) /* para +,-,*,/ esperam int */
        erro_semantico(no->linha, "operador %s exige operandos int", no->valor);

    return esq; /* tipo resultante */
}

static Tipo verifica_expr(AST *expr)
{
    if (!expr)
        return TIPO_INT;

    switch (expr->tipo)
    {
    case AST_INT:
        return TIPO_INT;
    case AST_CAR:
        return TIPO_CAR;

    case AST_ID:
    {
        Simbolo *s = buscarSimbolo(expr->valor);
        if (!s)
            erro_semantico(expr->linha, "identificador %s não declarado", expr->valor);
        return s->tipo;
    }

    case AST_ATRIB:
    {
        /* filho[0] é ID, filho[1] é expressão */
        AST *idNo = expr->filhos[0];
        Tipo tvar = verifica_expr(idNo); /* faz lookup e devolve tipo da var */
        Tipo texpr = verifica_expr(expr->filhos[1]);

        if (tvar != texpr)
            erro_semantico(expr->linha, "atribuição de tipos diferentes");

        return tvar;
    }

    case AST_OP:
    {
        /* operadores aritméticos ou relacionais / lógicos */
        if (!strcmp(expr->valor, "+") || !strcmp(expr->valor, "-") ||
            !strcmp(expr->valor, "*") || !strcmp(expr->valor, "/"))
        {
            return verifica_operador_binario(expr, TIPO_INT);
        }
        else if (!strcmp(expr->valor, "<") || !strcmp(expr->valor, ">") ||
                 !strcmp(expr->valor, "<=") || !strcmp(expr->valor, ">=") ||
                 !strcmp(expr->valor, "==") || !strcmp(expr->valor, "!="))
        {
            /* relacionais – operandos mesmo tipo, resultado int */
            Tipo esq = verifica_expr(expr->filhos[0]);
            Tipo dir = verifica_expr(expr->filhos[1]);
            if (esq != dir)
                erro_semantico(expr->linha, "operandos incompatíveis em %s", expr->valor);
            return TIPO_INT;
        }
        else if (!strcmp(expr->valor, "ou") || !strcmp(expr->valor, "e"))
        {
            /* lógicos – ambos int, resultado int */
            verifica_operador_binario(expr, TIPO_INT);
            return TIPO_INT;
        }
        else if (!strcmp(expr->valor, "uminus") || !strcmp(expr->valor, "!"))
        {
            Tipo t = verifica_expr(expr->filhos[0]);
            if (t != TIPO_INT)
                erro_semantico(expr->linha, "operador %s requer int", expr->valor);
            return TIPO_INT;
        }
        break;
    }

    case AST_CHAMADA_FUNCAO:
    {
        Simbolo *f = buscarSimbolo(expr->valor);
        if (!f || f->categoria != FUNCAO)
            erro_semantico(expr->linha, "função %s não declarada", expr->valor);

        /* conta argumentos passados */
        int n_args = (expr->n_filhos > 0 && expr->filhos[0])
                         ? expr->filhos[0]->n_filhos
                         : 0;
        if (n_args != f->numParametros)
            erro_semantico(expr->linha, "número de parâmetros incorreto em %s", expr->valor);

        /* verifica tipos de cada argumento (assume tabela tem ordem dos params) */
        if (n_args)
        {
            AST *lista = expr->filhos[0];
            for (int i = 0; i < n_args; ++i)
            {
                Tipo targ = verifica_expr(lista->filhos[i]);
                /* busca tipo do parâmetro formal i (percorrendo lista ligada dos símbolos) */
                /* simples: aceitar sempre, extensão opcional */
            }
        }
        return f->tipo;
    }

    default: /* expressões agrupadoras – só retorna tipo do último filho */
        if (expr->n_filhos)
            return verifica_expr(expr->filhos[expr->n_filhos - 1]);
    }
    return TIPO_INT; /* fallback */
}

/* ========================================================= */
/*  Verificação de comandos / blocos                         */
/* ========================================================= */

static void verifica_comando(AST *cmd)
{
    if (!cmd)
        return;

    switch (cmd->tipo)
    {
    case AST_ATRIB:
        verifica_expr(cmd); /* já valida */
        break;

    case AST_LEITURA:
    {
        AST *idNo = cmd->filhos[0];
        verifica_expr(idNo); /* só garante que existe */
        break;
    }

    case AST_ESCRITA:
        verifica_expr(cmd->filhos[0]);
        break;

    case AST_RETORNE:
    {
        Tipo t = verifica_expr(cmd->filhos[0]);
        if (t != tipoFuncaoAtual)
            erro_semantico(cmd->linha, "tipo do retorne diferente do tipo da função");
        break;
    }

    case AST_SE:
    {
        verifica_expr(cmd->filhos[0]);    /* condição */
        verifica_comando(cmd->filhos[1]); /* then */
        break;
    }

    case AST_SENAO:
    {
        AST *ifNode = cmd->filhos[0];
        verifica_comando(ifNode);
        verifica_comando(cmd->filhos[1]); /* else */
        break;
    }

    case AST_ENQUANTO:
    {
        verifica_expr(cmd->filhos[0]); /* condição */
        verifica_comando(cmd->filhos[1]);
        break;
    }

    case AST_BLOCO:
        percorre_bloco(cmd);
        break;

    case AST_LISTA_COMANDO:
        for (int i = 0; i < cmd->n_filhos; ++i)
            verifica_comando(cmd->filhos[i]);
        break;

    default:
        /* comando simples (;) já tratado */
        break;
    }
}

static void declara_variaveis(AST *listaDecl)
{
    if (!listaDecl)
        return;
    for (int i = 0; i < listaDecl->n_filhos; ++i)
    {
        AST *var = listaDecl->filhos[i];
        if (!var || var->tipo != AST_DECL_VARIAVEL) continue;
        if (!var)
            continue;
        if (var->tipo != AST_DECL_VARIAVEL)
            continue;
        if (!var->valor)
            erro_semantico(var->linha, "variável sem nome na AST");

        Tipo t = tipo_do_no(var->filhos[0]);
        if (buscarSimbolo(var->valor) && buscarSimbolo(var->valor)->categoria != FUNCAO)
            erro_semantico(var->linha, "identificador %s já declarado", var->valor);

        inserirVariavel(var->valor, t, 0);
    }
}

static void percorre_bloco(AST *bloco)
{
    novoEscopo();

    AST *decls = bloco->filhos[0];
    AST *stmts = bloco->filhos[1];

    declara_variaveis(decls);
    verifica_comando(stmts);

    removerEscopo();
}

/* ========================================================= */
/*  Verificação de funções e programa principal              */
/* ========================================================= */

static void verifica_funcao(AST *declFunc)
{
    const char *nome = declFunc->valor;
    Tipo retTipo = tipo_do_no(declFunc->filhos[0]);
    AST *fun = declFunc->filhos[1]; /* AST_FUNCAO */

    /* --- declaração na tabela, se ainda não existir --- */
    Simbolo *s = buscarSimbolo(nome);
    if (s)
        erro_semantico(declFunc->linha, "função %s duplicada", nome);

    s = inserirFuncao(nome, retTipo,
                      fun->filhos[0] ? fun->filhos[0]->n_filhos : 0);

    /* --- novo escopo para parâmetros + variáveis locais --- */
    novoEscopo();

    /* insere parâmetros na tabela */
    AST *listaParam = fun->filhos[0];
    if (listaParam)
        for (int i = 0; i < listaParam->n_filhos; ++i)
        {
            AST *param = listaParam->filhos[i]; /* AST_PARAM */
            Tipo tp = tipo_do_no(param->filhos[0]);
            const char *nomePar = param->filhos[1]->valor;

            if (buscarSimbolo(nomePar))
                erro_semantico(param->linha, "parâmetro %s duplicado", nomePar);

            inserirParametro(nomePar, tp, i, s);
        }

    /* corpo da função */
    tipoFuncaoAtual = retTipo;
    percorre_bloco(fun->filhos[1]); /* bloco */

    removerEscopo();
}

static void verifica_programa(AST *raiz)
{
    /* Estrutura: AST_PROGRAMA -> filhos[0] lista decl var/func
                                  filhos[1] programa principal */

    AST *listaDecl = raiz->filhos[0];
    AST *declProg = raiz->filhos[1];

    novoEscopo(); /* escopo global */

    /* --- var ou fun --- */
    if (listaDecl)
        for (int i = 0; i < listaDecl->n_filhos; ++i)
        {
            AST *item = listaDecl->filhos[i];
            if (item->tipo == AST_DECL_VARIAVEL)
            {
                const char *nome = item->valor;
                Tipo t = tipo_do_no(item->filhos[0]);
                if (!nome)
                    erro_semantico(item->linha, "variável global sem nome na AST (bug no parser)");
                if (buscarSimbolo(nome))
                    erro_semantico(item->linha, "variável %s duplicada", nome);
                inserirVariavel(nome, t, 0);
            }
            else if (item->tipo == AST_DECL_FUNCAO)
            {
                verifica_funcao(item);
            }
        }

    /* --- bloco principal 'programa' --- */
    tipoFuncaoAtual = TIPO_INT; /* não usado */
    percorre_bloco(declProg->filhos[0]);

    removerEscopo(); /* escopo global */
}

/* ========================================================= */
/*  Função pública                                           */
/* ========================================================= */

int analise_semanica(AST *raiz)
{
    if (!raiz)
        return 1;
    verifica_programa(raiz);
    return 1; /* sucesso – não houve exit(1) */
}
