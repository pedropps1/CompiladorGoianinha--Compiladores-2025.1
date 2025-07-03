#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"

static char *duplica(const char *s){
    size_t n = strlen(s) + 1;
    char *p  = malloc(n);
    if(p) memcpy(p,s,n);
    return p;
}

#define MAX_ESCOPOS 64
static TabelaSimbolos pilha[MAX_ESCOPOS];
static int topo = -1;

void novoEscopo(void){
    if(++topo >= MAX_ESCOPOS) exit(1);
    pilha[topo].lista = NULL;
}

void removerEscopo(void){
    if(topo < 0) return;
    Simbolo *s = pilha[topo].lista, *tmp;
    while(s){ tmp = s->prox; free(s->nome); free(s); s = tmp; }
    --topo;
}

static Simbolo *cria(const char *nome, Tipo t, Categoria c){
    Simbolo *s = malloc(sizeof *s);
    s->nome = duplica(nome);
    s->tipo = t; s->categoria = c;
    s->prox = pilha[topo].lista;
    pilha[topo].lista = s;
    return s;
}

void inserirVariavel(const char *nome, Tipo t, int pos){
    Simbolo *s = cria(nome,t,VARIAVEL);
    s->posicao = pos; s->numParametros = -1; s->funcaoRef = NULL;
}

Simbolo *inserirFuncao(const char *nome, Tipo t, int n){
    Simbolo *s = cria(nome,t,FUNCAO);
    s->posicao = -1; s->numParametros = n; s->funcaoRef = NULL;
    return s;
}

void inserirParametro(const char *nome, Tipo t, int pos, Simbolo *f){
    Simbolo *s = cria(nome,t,PARAMETRO);
    s->posicao = pos; s->numParametros = -1; s->funcaoRef = f;
}

Simbolo *buscarSimbolo(const char *nome){
    for(int i=topo;i>=0;--i)
        for(Simbolo *s = pilha[i].lista; s; s = s->prox)
            if(strcmp(s->nome,nome)==0) return s;
    return NULL;
}
