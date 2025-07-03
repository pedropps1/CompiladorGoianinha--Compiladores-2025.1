#ifndef AST_H
#define AST_H

#include <stdio.h>

/* ------------------------------------------------------------------
 * Tipos de nó da Árvore Sintática Abstrata (AST)
 * ------------------------------------------------------------------ */
typedef enum {
    /* Estruturas de alto nível */
    AST_PROGRAMA,
    AST_FUNCAO,              /* corpo de função (parâmetros + bloco)  */
    AST_DECL_FUNCAO,         /* declaração/definição de função        */
    AST_DECL_VARIAVEL,       /* declaração (var local ou global)      */

    /* Listas / agrupadores */
    AST_LISTA_DECL_VAR,
    AST_LISTA_COMANDO,
    AST_LISTA_PARAM,         /* lista de parâmetros formais           */
    AST_LISTA_EXPR,          /* lista de argumentos de chamada        */

    /* Blocos e comandos */
    AST_BLOCO,
    AST_COMANDO,             /* comando vazio ‘;’ (NOP)               */
    AST_ATRIB,
    AST_LEITURA,
    AST_ESCRITA,
    AST_RETORNE,
    AST_NOVALINHA,
    AST_SE,                  /* if sem else                           */
    AST_SENAO,               /* nó que agrupa if + else               */
    AST_ENQUANTO,

    /* Expressões / operandos */
    AST_OP,                  /* operador binário/unário               */
    AST_ID,
    AST_INT,
    AST_CAR,
    AST_STRING,
    AST_CHAMADA_FUNCAO,
    AST_PARAM,               /* parâmetro individual                  */

    /* Use este espaço para extensões futuras */
} ASTTipo;

/* ------------------------------------------------------------------
 * Estrutura do nó
 * ------------------------------------------------------------------ */
typedef struct AST {
    ASTTipo       tipo;
    char         *valor;     /* nome de id, operador, literal, etc.   */
    int           linha;     /* linha de origem no fonte              */
    int           n_filhos;  /* número de filhos                      */
    struct AST  **filhos;    /* vetor de ponteiros p/ filhos          */
} AST;

/* ------------------------------------------------------------------
 * API básica de manipulação da AST
 * ------------------------------------------------------------------ */
AST *ast_cria(ASTTipo tipo, const char *valor, int linha);
/* cria nó já com n filhos (varargs: n ponteiros AST*) */
AST *ast_cria_com_filhos(ASTTipo tipo, const char *valor,
                         int linha, int n, ...);
void ast_adiciona_filho(AST *pai, AST *filho);
void ast_libera(AST *a);
void ast_imprime(AST *a, int nivel);

#endif /* AST_H */
