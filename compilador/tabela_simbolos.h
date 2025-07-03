#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

typedef enum { TIPO_INT, TIPO_CAR } Tipo;
typedef enum { VARIAVEL, FUNCAO, PARAMETRO } Categoria;

typedef struct Simbolo {
    char      *nome;
    Tipo       tipo;
    Categoria  categoria;
    int        posicao;
    int        numParametros;
    struct Simbolo *funcaoRef;
    struct Simbolo *prox;
} Simbolo;

typedef struct TabelaSimbolos {
    Simbolo *lista;
} TabelaSimbolos;


void   novoEscopo(void);
void   removerEscopo(void);
void   inserirVariavel(const char *nome, Tipo t, int pos);
Simbolo* inserirFuncao(const char *nome, Tipo t, int nParams);
void   inserirParametro(const char *nome, Tipo t, int pos, Simbolo *funcao);
Simbolo* buscarSimbolo(const char *nome);

#endif
