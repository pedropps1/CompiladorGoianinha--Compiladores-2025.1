/* ===================================================================== *
 * codigo.c  ─  Geração de Assembly-MIPS (SPIM) para a linguagem
 * Goianinha – VERSÃO CORRIGIDA COM SUPORTE A VARIÁVEIS LOCAIS
 * ===================================================================== */
#include "codigo.h"
#include "tabela_simbolos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h> // Para INT_MAX

// --- ESTRUTURAS PARA GERENCIAMENTO MULTI-PASSADA ---

// Para a lista de strings literais encontradas na primeira passada
typedef struct StringLiteral {
    char* valor;
    char label[32];
    struct StringLiteral* next;
} StringLiteral;

static StringLiteral* lista_strings = NULL;
static int string_id_counter = 0;

// --- PROTÓTIPOS INTERNOS ---

static void gera_funcao(AST *decl);
static const char *gera_expr(AST *e);
static void gera_comando(AST *c, const char *rotulo_saida_func);
static void coleta_strings_pass(AST *no);

/* ------------------------------------------------------------------ */
/* Emissão e Rótulos                                                  */
/* ------------------------------------------------------------------ */
static FILE *out;
static int rotulo_id = 0;

static void emit(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void emit(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
}

static void novo_rotulo(char *buffer, int max_size) {
    snprintf(buffer, max_size, "L%d", rotulo_id++);
}

// Gera um nome de label seguro para funções (ex: user_minhaFunc)
static void gera_nome_label_func(const char* nome_original, char* buffer_saida, int max_size) {
    if (strcmp(nome_original, "programa") == 0) {
        strncpy(buffer_saida, "main", max_size);
    } else {
        snprintf(buffer_saida, max_size, "user_%s", nome_original);
    }
}

// Gera um nome de label seguro para variáveis globais
static void gera_nome_label_var(const char* nome_original, char* buffer_saida, int max_size) {
    snprintf(buffer_saida, max_size, "var_%s", nome_original);
}


/* ------------------------------------------------------------------ */
/* Gerenciamento de Símbolos no Frame (Parâmetros e Locais)           */
/* ------------------------------------------------------------------ */
#define MAX_FRAME_SYMBOLS 64
#define WORD_SIZE 4
static struct { char nome[32]; int offset; } frame_map[MAX_FRAME_SYMBOLS];
static int frame_map_size = 0;

static void frame_map_init(void) { frame_map_size = 0; }

static void frame_map_add(const char *n, int off) {
    if (frame_map_size < MAX_FRAME_SYMBOLS) {
        strncpy(frame_map[frame_map_size].nome, n, 31);
        frame_map[frame_map_size].nome[31] = '\0';
        frame_map[frame_map_size].offset = off;
        ++frame_map_size;
    }
}

// Retorna offset em caso de sucesso, ou INT_MAX em caso de falha
static int frame_map_get_offset(const char *n) {
    for (int i = 0; i < frame_map_size; ++i)
        if (strcmp(frame_map[i].nome, n) == 0) return frame_map[i].offset;
    return INT_MAX; // Sentinela para não encontrado
}


static const char *TREG[] = {"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$t8","$t9"};
#define NTEMP 10
static int topo_temp = 0;

static const char *talloc(void) {
    if (topo_temp >= NTEMP) {
        fprintf(stderr, "ERRO: Esgotou registradores temporários.\n");
        exit(1);
    }
    return TREG[topo_temp++];
}
static void tfree(void) { if (topo_temp > 0) --topo_temp; }


/* ------------------------------------------------------------------ */
/* Passada 1: Coleta de Strings Literais                              */
/* ------------------------------------------------------------------ */

static void adiciona_string_a_lista(const char* valor) {
    for (StringLiteral* p = lista_strings; p != NULL; p = p->next) {
        if (strcmp(p->valor, valor) == 0) return;
    }
    StringLiteral* novo = malloc(sizeof(StringLiteral));
    novo->valor = strdup(valor);
    snprintf(novo->label, sizeof(novo->label), "str%d", string_id_counter++);
    novo->next = lista_strings;
    lista_strings = novo;
}

static void coleta_strings_pass(AST* no) {
    if (!no) return;
    if (no->tipo == AST_STRING) {
        adiciona_string_a_lista(no->valor);
    }
    for (int i = 0; i < no->n_filhos; ++i) {
        coleta_strings_pass(no->filhos[i]);
    }
}

/* ------------------------------------------------------------------ */
/* Passada 2: Geração de Código                                       */
/* ------------------------------------------------------------------ */

static void gera_secao_data(AST* raiz) {
    emit(".data\n");
    emit("nl: .asciiz \"\n\"\n");

    for (StringLiteral* p = lista_strings; p != NULL; p = p->next) {
        emit("%s: .asciiz %s\n", p->label, p->valor);
    }

    if (raiz->n_filhos > 0 && raiz->filhos[0]) {
        AST *lista = raiz->filhos[0];
        char var_label[256];
        for (int i = 0; i < lista->n_filhos; ++i) {
            AST *item = lista->filhos[i];
            if (item->tipo == AST_DECL_VARIAVEL) {
                gera_nome_label_var(item->valor, var_label, sizeof(var_label));
                emit("%s: .word 0\n", var_label);
            }
        }
    }
}

static const char* obter_label_string(const char* valor) {
    for (StringLiteral* p = lista_strings; p != NULL; p = p->next) {
        if (strcmp(p->valor, valor) == 0) return p->label;
    }
    return NULL;
}


static const char *gera_expr(AST *e) {
    if (!e) return NULL;
    switch (e->tipo) {
        case AST_INT: {
            const char *t = talloc();
            emit("    li %s, %s\n", t, e->valor);
            return t;
        }
        case AST_CAR: {
            const char *t = talloc();
            emit("    li %s, %d\n", t, e->valor[1]);
            return t;
        }
        case AST_STRING: {
            const char *t = talloc();
            const char *label_str = obter_label_string(e->valor);
            emit("    la %s, %s\n", t, label_str);
            return t;
        }
        case AST_ID: {
            const char *t = talloc();
            int off = frame_map_get_offset(e->valor);
            if (off != INT_MAX) {
                emit("    lw %s, %d($fp)\n", t, off);
            } else {
                char var_label[256];
                gera_nome_label_var(e->valor, var_label, sizeof(var_label));
                emit("    lw %s, %s\n", t, var_label);
            }
            return t;
        }
        case AST_ATRIB: {
            const char *rhs = gera_expr(e->filhos[1]);
            const char *id_name = e->filhos[0]->valor;
            int off = frame_map_get_offset(id_name);
            if (off != INT_MAX) {
                emit("    sw %s, %d($fp)\n", rhs, off);
            } else {
                char var_label[256];
                gera_nome_label_var(id_name, var_label, sizeof(var_label));
                emit("    sw %s, %s\n", rhs, var_label);
            }
            return rhs;
        }
        case AST_OP: {
            const char *a = gera_expr(e->filhos[0]);
            if (e->n_filhos == 1) {
                 if (!strcmp(e->valor, "uminus")) emit("    sub %s, $zero, %s\n", a, a);
                 else if (!strcmp(e->valor, "!")) emit("    seq %s, %s, $zero\n", a, a);
                 return a;
            }
            const char *b = gera_expr(e->filhos[1]);
            if (!strcmp(e->valor, "+")) emit("    add %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "-")) emit("    sub %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "*")) emit("    mul %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "/")) { emit("    div %s, %s\n", a, b); emit("    mflo %s\n", a); }
            else if (!strcmp(e->valor, "<")) emit("    slt %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, ">")) emit("    sgt %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "<=")) emit("    sle %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, ">=")) emit("    sge %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "==")) emit("    seq %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "!=")) emit("    sne %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "e")) emit("    and %s, %s, %s\n", a, a, b);
            else if (!strcmp(e->valor, "ou")) emit("    or %s, %s, %s\n", a, a, b);
            tfree();
            return a;
        }
        case AST_CHAMADA_FUNCAO: {
            int n_args = (e->n_filhos > 0 && e->filhos[0]) ? e->filhos[0]->n_filhos : 0;

            // Salvar registradores temporários em uso
            int regs_em_uso = topo_temp;
            if (regs_em_uso > 0) {
                emit("    # Salvando %d temporários antes da chamada a '%s'\n", regs_em_uso, e->valor);
                emit("    addi $sp, $sp, -%d\n", regs_em_uso * WORD_SIZE);
                for (int i = 0; i < regs_em_uso; ++i) {
                    emit("    sw   %s, %d($sp)\n", TREG[i], i * WORD_SIZE);
                }
            }

            // Avaliar e passar argumentos
            for (int i = 0; i < n_args && i < 4; ++i) {
                const char *r = gera_expr(e->filhos[0]->filhos[i]);
                emit("    move $a%d, %s\n", i, r);
                tfree();
            }

            // Chamar a função
            char label_chamada[256];
            gera_nome_label_func(e->valor, label_chamada, sizeof(label_chamada));
            emit("    jal  %s\n", label_chamada);

            // Restaurar registradores temporários
            if (regs_em_uso > 0) {
                emit("    # Restaurando %d temporários após a chamada\n", regs_em_uso);
                for (int i = 0; i < regs_em_uso; ++i) {
                    emit("    lw   %s, %d($sp)\n", TREG[i], i * WORD_SIZE);
                }
                emit("    addi $sp, $sp, %d\n", regs_em_uso * WORD_SIZE);
            }

            // Mover valor de retorno para um novo temporário
            const char *ret = talloc();
            emit("    move %s, $v0\n", ret);
            return ret;
        }
        default: break;
    }
    return NULL;
}

static void gera_comando(AST *c, const char *rotulo_saida_func) {
    if (!c) return;
    switch (c->tipo) {
        case AST_LISTA_COMANDO:
            for (int i = 0; i < c->n_filhos; ++i)
                gera_comando(c->filhos[i], rotulo_saida_func);
            break;
        case AST_ATRIB:
        case AST_CHAMADA_FUNCAO:
            gera_expr(c);
            tfree();
            break;
        case AST_LEITURA: {
            emit("    li $v0, 5\n"); // syscall 5 para ler inteiro
            emit("    syscall\n");
            const char *id_name = c->filhos[0]->valor;
            int off = frame_map_get_offset(id_name);
            if (off != INT_MAX) {
                emit("    sw $v0, %d($fp)\n", off);
            } else {
                char var_label[256];
                gera_nome_label_var(id_name, var_label, sizeof(var_label));
                emit("    sw $v0, %s\n", var_label);
            }
            break;
        }
        case AST_ESCRITA: {
            AST *expr = c->filhos[0];
            const char *reg = gera_expr(expr);
            emit("    move $a0, %s\n", reg);
            if (expr->tipo == AST_STRING) {
                emit("    li $v0, 4\n");
            } else if (expr->tipo == AST_CAR) {
                emit("    li $v0, 11\n");
            } else {
                emit("    li $v0, 1\n");
            }
            emit("    syscall\n");
            tfree();
            break;
        }
        case AST_NOVALINHA:
            emit("    la $a0, nl\n");
            emit("    li $v0, 4\n");
            emit("    syscall\n");
            break;
        case AST_RETORNE: {
            const char *r = gera_expr(c->filhos[0]);
            emit("    move $v0, %s\n", r);
            tfree();
            emit("    j %s\n", rotulo_saida_func);
            break;
        }
        case AST_SE: {
            char rot_fim[32]; novo_rotulo(rot_fim, sizeof(rot_fim));
            const char *cond = gera_expr(c->filhos[0]);
            emit("    beq %s, $zero, %s\n", cond, rot_fim);
            tfree();
            gera_comando(c->filhos[1], rotulo_saida_func);
            emit("%s:\n", rot_fim);
            break;
        }
        case AST_SENAO: {
            AST *no_se = c->filhos[0];
            char rot_senao[32], rot_fim[32];
            novo_rotulo(rot_senao, sizeof(rot_senao));
            novo_rotulo(rot_fim, sizeof(rot_fim));
            const char *cond = gera_expr(no_se->filhos[0]);
            emit("    beq %s, $zero, %s\n", cond, rot_senao);
            tfree();
            gera_comando(no_se->filhos[1], rotulo_saida_func);
            emit("    j %s\n", rot_fim);
            emit("%s:\n", rot_senao);
            gera_comando(c->filhos[1], rotulo_saida_func);
            emit("%s:\n", rot_fim);
            break;
        }
        case AST_ENQUANTO: {
            char rot_inicio[32], rot_fim[32];
            novo_rotulo(rot_inicio, sizeof(rot_inicio));
            novo_rotulo(rot_fim, sizeof(rot_fim));
            emit("%s:\n", rot_inicio);
            const char *cond = gera_expr(c->filhos[0]);
            emit("    beq %s, $zero, %s\n", cond, rot_fim);
            tfree();
            gera_comando(c->filhos[1], rotulo_saida_func);
            emit("    j %s\n", rot_inicio);
            emit("%s:\n", rot_fim);
            break;
        }
        case AST_BLOCO:
            if(c->n_filhos > 1 && c->filhos[1]) {
                 gera_comando(c->filhos[1], rotulo_saida_func);
            }
            break;
        default: break;
    }
}

static void gera_funcao(AST *decl) {
    const char *nome_original = decl->valor;
    AST *bloco = NULL, *listaParam = NULL;

    if (decl->n_filhos > 1 && decl->filhos[1]->tipo == AST_FUNCAO) {
        AST *func = decl->filhos[1];
        listaParam = func->filhos[0];
        bloco = func->filhos[1];
    } else if (decl->n_filhos > 0 && decl->filhos[0]->tipo == AST_BLOCO) {
        bloco = decl->filhos[0];
    } else {
        fprintf(stderr, "ERRO: Declaração de função '%s' malformada.\n", nome_original);
        exit(1);
    }
    
    char label_func[256];
    char rotulo_saida[32];
    gera_nome_label_func(nome_original, label_func, sizeof(label_func));
    novo_rotulo(rotulo_saida, sizeof(rotulo_saida));

    int n_params = (listaParam) ? listaParam->n_filhos : 0;
    AST* lista_decl_locais = (bloco && bloco->n_filhos > 0) ? bloco->filhos[0] : NULL;
    int n_locals = (lista_decl_locais && lista_decl_locais->tipo == AST_LISTA_DECL_VAR) ? lista_decl_locais->n_filhos : 0;
    int frame_size = (2 * WORD_SIZE) + (n_params * WORD_SIZE) + (n_locals * WORD_SIZE);

    emit("\n.globl %s\n.text\n%s:\n", label_func, label_func);
    if (strcmp(nome_original, "programa") == 0) emit("programa:\n");

    emit("    addi $sp, $sp, -%d\n", frame_size);
    emit("    sw   $ra, 0($sp)\n");
    emit("    sw   $fp, 4($sp)\n");
    emit("    move $fp, $sp\n");

    frame_map_init();
    if (listaParam) {
        int off = 8;
        for (int i = 0; i < listaParam->n_filhos; ++i) {
            AST *param = listaParam->filhos[i];
            const char *nomePar = param->filhos[1]->valor;
            if (i < 4) emit("    sw   $a%d, %d($fp)\n", i, off);
            frame_map_add(nomePar, off);
            off += WORD_SIZE;
        }
    }

    if (n_locals > 0) {
        int off = -8;
        for (int i = 0; i < n_locals; ++i) {
            AST *var_decl = lista_decl_locais->filhos[i];
            const char *nomeVar = var_decl->valor;
            frame_map_add(nomeVar, off);
            off -= WORD_SIZE;
        }
    }

    gera_comando(bloco, rotulo_saida);

    emit("%s:\n", rotulo_saida);
    emit("    # Epílogo\n");
    emit("    lw $ra, 0($sp)\n");
    emit("    lw $fp, 4($sp)\n");
    emit("    addi $sp, $sp, %d\n", frame_size);
    emit("    jr $ra\n");
}

/* ------------------------------------------------------------------ */
/* API Principal                                                      */
/* ------------------------------------------------------------------ */
int gerar_codigo_mips(AST *raiz, const char *nome_s) {
    if (!raiz) return 0;
    out = fopen(nome_s, "w");
    if (!out) {
        perror(nome_s);
        return 0;
    }
    
    coleta_strings_pass(raiz);
    
    gera_secao_data(raiz);
    
    emit("\n.text\n");
    
    if (raiz->n_filhos > 0 && raiz->filhos[0]) {
        AST* lista_decl = raiz->filhos[0];
        for (int i = 0; i < lista_decl->n_filhos; ++i) {
            if (lista_decl->filhos[i]->tipo == AST_DECL_FUNCAO) {
                gera_funcao(lista_decl->filhos[i]);
            }
        }
    }
    if (raiz->n_filhos > 1 && raiz->filhos[1]) {
        gera_funcao(raiz->filhos[1]);
    }
    
    while (lista_strings) {
        StringLiteral* temp = lista_strings;
        lista_strings = lista_strings->next;
        free(temp->valor);
        free(temp);
    }
    
    fclose(out);
    return 1;
}
