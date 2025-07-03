#include <stdio.h>
#include "ast.h"
#include "semantico.h"
#include "codigo.h" // Adicionar a inclusão para gerar_codigo_mips

// "arvore_raiz" é definida em goianinha.y
extern AST *arvore_raiz;

extern FILE *yyin;
int yyparse(void);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Uso: %s <fonte.go>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin)
    {
        perror("falha abrindo arquivo");
        return 1;
    }

    int r = yyparse();
    if (r == 0)
    {
        if (arvore_raiz)
        {
            puts("--------- AST ---------");
            ast_imprime(arvore_raiz, 0);
        }

        if (analise_semanica(arvore_raiz))
        {
            puts("Compilado com sucesso (fase semântica)");
            
            if (gerar_codigo_mips(arvore_raiz, "saida.s"))
            {
                puts("Compilado com sucesso (fase semântica + código)");
            }
            else
            {
                fprintf(stderr, "ERRO: Falha ao gerar o código de saída.\n");
                r = 1;
            }
        }
        else
        {
            fprintf(stderr, "ERRO: Falha na análise semântica.\n");
            r = 1;
        }

        ast_libera(arvore_raiz);
    }
    return r;
}