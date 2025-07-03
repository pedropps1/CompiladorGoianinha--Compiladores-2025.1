#include "ast.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Cria nó sem filhos
AST *ast_cria(ASTTipo tipo, const char *valor, int linha) {
    AST *a = (AST *)malloc(sizeof(AST));
    a->tipo = tipo;
    a->valor = valor ? strdup(valor) : NULL;
    a->linha = linha;
    a->n_filhos = 0;
    a->filhos = NULL;
    return a;
}

// Cria nó já com n filhos passados via argumentos variádicos
AST *ast_cria_com_filhos(ASTTipo tipo, const char *valor, int linha, int n, ...) {
    AST *a = ast_cria(tipo, valor, linha);
    if(n > 0) {
        a->filhos = (AST **)malloc(n * sizeof(AST *));
        a->n_filhos = n;
        va_list ap;
        va_start(ap, n);
        for(int i=0; i<n; i++)
            a->filhos[i] = va_arg(ap, AST *);
        va_end(ap);
    }
    return a;
}

// Adiciona filho (realoca o vetor de filhos)
void ast_adiciona_filho(AST *pai, AST *filho) {
    pai->filhos = (AST **)realloc(pai->filhos, (pai->n_filhos + 1) * sizeof(AST *));
    pai->filhos[pai->n_filhos++] = filho;
}

// Libera recursivamente
void ast_libera(AST *a) {
    if(!a) return;
    for(int i=0; i<a->n_filhos; i++)
        ast_libera(a->filhos[i]);
    free(a->filhos);
    free(a->valor);
    free(a);
}

// Imprime AST (indentado)
void ast_imprime(AST *a, int nivel) {
    if(!a) return;
    for(int i=0; i<nivel; i++) printf("  ");
    printf("[%d] ", a->tipo);
    if(a->valor) printf("'%s' ", a->valor);
    printf("(linha %d)\n", a->linha);
    for(int i=0; i<a->n_filhos; i++)
        ast_imprime(a->filhos[i], nivel+1);
}
