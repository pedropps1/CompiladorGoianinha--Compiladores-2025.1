/* ------------------------------------------------------------------
 * codigo.h   –  Interface da geração de código MIPS para Goianinha
 * ------------------------------------------------------------------ */
#ifndef CODIGO_H
#define CODIGO_H

#include "ast.h"

int gerar_codigo_mips(AST *raiz, const char *nome_asm);

#endif /* CODIGO_H */
