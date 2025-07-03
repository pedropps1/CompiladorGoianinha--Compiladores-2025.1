/* ------------------------------------------------------------------
 * goianinha.y  –  Parser + AST para a linguagem Goianinha
 * ------------------------------------------------------------------ */

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "ast.h"

    extern int yylineno;        /* linha atual do Flex */
    int yylex(void);
    void yyerror(const char *s);

    AST *arvore_raiz = NULL;    /* raiz global da AST */

    void yyerror(const char *s) {
        fprintf(stderr, "ERRO: %s linha %d\n", s, yylineno);
    }
%}

/* ---------- %union ---------- */
%union {
    int   ival;
    char *sval;
    AST  *ast;
}

/* ---------- Tokens ---------- */
%token PROGRAMA INT CAR RETORNE LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO ENQUANTO EXECUTE
%token OU E
%token EQ NEQ LEQ GEQ          /* ==  !=  <=  >= */
%token <sval> ID
%token <sval> INTCONST
%token <sval> CARCONST
%token <sval> STRING

/* ---------- Precedência ---------- */
%right '='
%left OU
%left E
%left EQ NEQ
%left '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/'
%right UMINUS '!'

/* ---------- Tipagem de não‐terminais ---------- */
%type <ast> Programa DeclFuncVar DeclFunc DeclProg
%type <ast> ListaDeclVar DeclVar
%type <ast> ListaParametros ListaParametrosCont
%type <ast> Bloco ListaComando Comando
%type <ast> Expr OrExpr AndExpr EqExpr DesigExpr
%type <ast> AddExpr MulExpr UnExpr PrimExpr
%type <ast> ListExpr Tipo

%%

/* ====================================================== */
Programa
    : DeclFuncVar DeclProg
        {
            $$ = ast_cria_com_filhos(AST_PROGRAMA,NULL,@1.first_line,2,$1,$2);
            arvore_raiz = $$;
        }
    ;

/* ====================================================== */
/*   Declarações globais (variáveis/funções)              */
/* ====================================================== */
DeclFuncVar
    /* lista vazia → nó LISTA vazio, nunca NULL */
    : /* ε */
        { $$ = ast_cria(AST_LISTA_DECL_VAR,NULL,yylineno); }

    /* var global */
    | DeclFuncVar Tipo ID DeclVar ';'
    {
        /* primeiro id */
        AST *var0 = ast_cria(AST_DECL_VARIAVEL, $3, @3.first_line);
        ast_adiciona_filho(var0, $2);   /* usa o nó $2 original */

        AST *lista = $1 ? $1 : ast_cria(AST_LISTA_DECL_VAR, NULL, @3.first_line);
        ast_adiciona_filho(lista, var0);

        /* demais ids vindos de DeclVar */
        if ($4)
            for (int i = 0; i < $4->n_filhos; ++i) {
                AST *v = $4->filhos[i];
                /* ---- CÓPIA do nó de tipo ---- */
                AST *tipo_copia = ast_cria($2->tipo, $2->valor, $2->linha);
                ast_adiciona_filho(v, tipo_copia);
                ast_adiciona_filho(lista, v);
            }
        $$ = lista;
    }

    /* função global */
    | DeclFuncVar Tipo ID DeclFunc
        {
            AST *func = ast_cria(AST_DECL_FUNCAO,$3,@3.first_line);
            ast_adiciona_filho(func,$2);      /* tipo retorno */
            ast_adiciona_filho(func,$4);      /* corpo        */

            AST *lista = $1 ? $1 : ast_cria(AST_LISTA_DECL_VAR,NULL,@1.first_line);
            ast_adiciona_filho(lista,func);
            $$ = lista;
        }
    ;

/* cabeçalho+corpo da função */
DeclFunc
    : '(' ListaParametros ')' Bloco
        { $$ = ast_cria_com_filhos(AST_FUNCAO,NULL,@1.first_line,2,$2,$4); }
    ;

/* ====================================================== */
/*   Declarações locais                                   */
/* ====================================================== */
ListaDeclVar
    : /* ε */
        { $$ = ast_cria(AST_LISTA_DECL_VAR,NULL,yylineno); }

    | ListaDeclVar Tipo ID DeclVar ';'
    {
        AST *var0 = ast_cria(AST_DECL_VARIAVEL,$3,@3.first_line);
        ast_adiciona_filho(var0,$2);

        AST *lista = $1 ? $1 : ast_cria(AST_LISTA_DECL_VAR,NULL,@3.first_line);
        ast_adiciona_filho(lista,var0);

        if ($4)
            for (int i=0;i<$4->n_filhos;++i){
                AST *v = $4->filhos[i];
                AST *tipo_copia = ast_cria($2->tipo,$2->valor,$2->linha);
                ast_adiciona_filho(v,tipo_copia);
                ast_adiciona_filho(lista,v);
            }
        $$ = lista;
    }
    ;

DeclVar
    : /* ε */
        { $$ = ast_cria(AST_LISTA_DECL_VAR, NULL, yylineno); }

    | ',' ID DeclVar
        {
            AST *var = ast_cria(AST_DECL_VARIAVEL, $2, @2.first_line);
            AST *lista = $3 ? $3 : ast_cria(AST_LISTA_DECL_VAR, NULL, @2.first_line);
            ast_adiciona_filho(lista, var);
            $$ = lista;
        }
    ;

/* ====================================================== */
Bloco
    : '{' ListaDeclVar ListaComando '}'
        {
            $$ = ast_cria_com_filhos(AST_BLOCO,NULL,@1.first_line,2,$2,$3);
        }
    ;

/* lista de comandos */
ListaComando
    : /* ε */
        { $$ = ast_cria(AST_LISTA_COMANDO,NULL,yylineno); }

    | ListaComando Comando
        {
            AST *lista = $1 ? $1 : ast_cria(AST_LISTA_COMANDO,NULL,@2.first_line);
            ast_adiciona_filho(lista,$2);
            $$ = lista;
        }
    ;

/* ====================================================== */
/*   Comandos                                             */
/* ====================================================== */
Comando
    : ';'                    { $$ = ast_cria(AST_COMANDO,";",@1.first_line); }

    | Expr ';'               { $$ = $1; }

    | RETORNE Expr ';'
        { $$ = ast_cria_com_filhos(AST_RETORNE,NULL,@1.first_line,1,$2); }

    | LEIA ID ';'
        {
            AST *id = ast_cria(AST_ID,$2,@2.first_line);
            $$ = ast_cria_com_filhos(AST_LEITURA,NULL,@1.first_line,1,id);
        }

    | ESCREVA Expr ';'
        { $$ = ast_cria_com_filhos(AST_ESCRITA,NULL,@1.first_line,1,$2); }

    | ESCREVA STRING ';'
        {
            AST *str = ast_cria(AST_STRING,$2,@2.first_line);
            $$ = ast_cria_com_filhos(AST_ESCRITA,NULL,@1.first_line,1,str);
        }

    | NOVALINHA ';'
        { $$ = ast_cria(AST_NOVALINHA,NULL,@1.first_line); }

    | SE '(' Expr ')' ENTAO Comando
        { $$ = ast_cria_com_filhos(AST_SE,NULL,@1.first_line,2,$3,$6); }

    | SE '(' Expr ')' ENTAO Comando SENAO Comando
        {
            AST *ifnode = ast_cria_com_filhos(AST_SE,NULL,@1.first_line,2,$3,$6);
            $$ = ast_cria_com_filhos(AST_SENAO,NULL,@1.first_line,2,ifnode,$8);
        }

    | ENQUANTO '(' Expr ')' EXECUTE Comando
        { $$ = ast_cria_com_filhos(AST_ENQUANTO,NULL,@1.first_line,2,$3,$6); }

    | Bloco
        { $$ = $1; }
    ;

/* ====================================================== */
/*   Expressões                                           */
/* ====================================================== */
Expr
    : OrExpr
        { $$ = $1; }

    | ID '=' Expr
        {
            AST *id = ast_cria(AST_ID,$1,@1.first_line);
            $$ = ast_cria_com_filhos(AST_ATRIB,"=",@2.first_line,2,id,$3);
        }
    ;

OrExpr
    : OrExpr OU AndExpr
        { $$ = ast_cria_com_filhos(AST_OP,"ou",@2.first_line,2,$1,$3); }
    | AndExpr
        { $$ = $1; }
    ;

AndExpr
    : AndExpr E EqExpr
        { $$ = ast_cria_com_filhos(AST_OP,"e",@2.first_line,2,$1,$3); }
    | EqExpr
        { $$ = $1; }
    ;

EqExpr
    : EqExpr EQ  DesigExpr   { $$ = ast_cria_com_filhos(AST_OP,"==",@2.first_line,2,$1,$3); }
    | EqExpr NEQ DesigExpr   { $$ = ast_cria_com_filhos(AST_OP,"!=",@2.first_line,2,$1,$3); }
    | DesigExpr              { $$ = $1; }
    ;

DesigExpr
    : DesigExpr '<'  AddExpr { $$ = ast_cria_com_filhos(AST_OP,"<",@2.first_line,2,$1,$3); }
    | DesigExpr '>'  AddExpr { $$ = ast_cria_com_filhos(AST_OP,">",@2.first_line,2,$1,$3); }
    | DesigExpr LEQ AddExpr  { $$ = ast_cria_com_filhos(AST_OP,"<=",@2.first_line,2,$1,$3); }
    | DesigExpr GEQ AddExpr  { $$ = ast_cria_com_filhos(AST_OP,">=",@2.first_line,2,$1,$3); }
    | AddExpr               { $$ = $1; }
    ;

AddExpr
    : AddExpr '+' MulExpr   { $$ = ast_cria_com_filhos(AST_OP,"+",@2.first_line,2,$1,$3); }
    | AddExpr '-' MulExpr   { $$ = ast_cria_com_filhos(AST_OP,"-",@2.first_line,2,$1,$3); }
    | MulExpr               { $$ = $1; }
    ;

MulExpr
    : MulExpr '*' UnExpr    { $$ = ast_cria_com_filhos(AST_OP,"*",@2.first_line,2,$1,$3); }
    | MulExpr '/' UnExpr    { $$ = ast_cria_com_filhos(AST_OP,"/",@2.first_line,2,$1,$3); }
    | UnExpr                { $$ = $1; }
    ;

UnExpr
    : '-' UnExpr   %prec UMINUS { $$ = ast_cria_com_filhos(AST_OP,"uminus",@1.first_line,1,$2); }
    | '!' UnExpr                { $$ = ast_cria_com_filhos(AST_OP,"!",@1.first_line,1,$2); }
    | PrimExpr                  { $$ = $1; }
    ;

PrimExpr
    : ID '(' ListExpr ')'
        {
            AST *call = ast_cria(AST_CHAMADA_FUNCAO,$1,@1.first_line);
            if ($3) ast_adiciona_filho(call,$3);
            $$ = call;
        }
    | ID '(' ')'                { $$ = ast_cria(AST_CHAMADA_FUNCAO,$1,@1.first_line); }
    | ID                        { $$ = ast_cria(AST_ID,$1,@1.first_line); }
    | CARCONST                  { $$ = ast_cria(AST_CAR,$1,@1.first_line); }
    | INTCONST                  { $$ = ast_cria(AST_INT,$1,@1.first_line); }
    | '(' Expr ')'              { $$ = $2; }
    ;

ListExpr
    : Expr
        { $$ = ast_cria_com_filhos(AST_LISTA_EXPR,NULL,@1.first_line,1,$1); }
    | ListExpr ',' Expr
        {
            $$ = $1;
            ast_adiciona_filho($$,$3);
        }
    ;

/* ====================================================== */
/*   Tipos                                                */
/* ====================================================== */
Tipo
    : INT   { $$ = ast_cria(AST_INT,"int",@1.first_line); }
    | CAR   { $$ = ast_cria(AST_CAR,"car",@1.first_line); }
    ;

/* ====================================================== */
/*   Lista de parâmetros formais                          */
/* ====================================================== */
ListaParametros
    : /* ε */
        { $$ = ast_cria(AST_LISTA_PARAM,NULL,yylineno); }
    | ListaParametrosCont
        { $$ = $1; }
    ;

ListaParametrosCont
    : Tipo ID
        {
            AST *id = ast_cria(AST_ID,$2,@2.first_line);
            AST *par = ast_cria_com_filhos(AST_PARAM,NULL,@1.first_line,2,$1,id);
            $$ = ast_cria_com_filhos(AST_LISTA_PARAM,NULL,@1.first_line,1,par);
        }
    | ListaParametrosCont ',' Tipo ID
        {
            AST *id = ast_cria(AST_ID,$4,@4.first_line);
            AST *par = ast_cria_com_filhos(AST_PARAM,NULL,@3.first_line,2,$3,id);
            $$ = $1;
            ast_adiciona_filho($$,par);
        }
    ;

/* ====================================================== */
/*   Declaração do bloco principal "programa"             */
/* ====================================================== */
DeclProg
    : PROGRAMA Bloco
        { $$ = ast_cria_com_filhos(AST_DECL_FUNCAO,"programa",@1.first_line,1,$2); }
    ;

%%

/* Nenhum main aqui – usamos main.c separado */
