%{
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ast.h"
#include "goianinha.tab.h"

int linhacomentario = 0;

#define YY_USER_ACTION  yylloc.first_line = yylloc.last_line = yylineno;
%}

%option yylineno
%option noyywrap
%x COMENTARIO

DIGITO      [0-9]
LETRA       [a-zA-Z_]
ID          {LETRA}({LETRA}|{DIGITO})*
CARCONST    \'([^\'\n])\'
STRINGLIT   \"([^\"\n]|\\.)*\"

%%

"programa"          { return PROGRAMA; }
"int"               { return INT; }
"car"               { return CAR; }
"retorne"           { return RETORNE; }
"leia"              { return LEIA; }
"escreva"           { return ESCREVA; }
"novalinha"         { return NOVALINHA; }
"se"                { return SE; }
"entao"             { return ENTAO; }
"senao"             { return SENAO; }
"enquanto"          { return ENQUANTO; }
"execute"           { return EXECUTE; }
"ou"                { return OU; }
"e"                 { return E; }

"=="                { return EQ; }
"!="                { return NEQ; }
"<="                { return LEQ; }
">="                { return GEQ; }

"+"  { return '+'; }
"-"  { return '-'; }
"*"  { return '*'; }
"/"  { return '/'; }
"("  { return '('; }
")"  { return ')'; }
"{"  { return '{'; }
"}"  { return '}'; }
";"  { return ';'; }
","  { return ','; }
"<"  { return '<'; }
">"  { return '>'; }
"="  { return '='; }
"!"  { return '!'; }

{ID}          { yylval.sval = strdup(yytext); return ID; }
{DIGITO}+     { yylval.sval = strdup(yytext); return INTCONST; }
{CARCONST}    { yylval.sval = strdup(yytext); return CARCONST; }
{STRINGLIT}   { yylval.sval = strdup(yytext); return STRING; }

\n                  ;

"/*"                { BEGIN(COMENTARIO); linhacomentario = yylineno; }

<COMENTARIO>"*/"    { BEGIN(INITIAL); }

<COMENTARIO>\n      ;       

<COMENTARIO>.       ;
<COMENTARIO><<EOF>> {
    fprintf(stderr,"ERRO: COMENTARIO NAO TERMINA %d\n", linhacomentario);
    BEGIN(INITIAL);
}

\"([^\"\n]|\\.)*\n  {
    fprintf(stderr,"ERRO: cadeia de caracteres nao termina na linha %d\n", yylineno);
}

[ \t\r]+            ;

.                   { fprintf(stderr,"ERRO: CARACTERE INVALIDO %d\n", yylineno); }

<<EOF>>             { return 0; }
