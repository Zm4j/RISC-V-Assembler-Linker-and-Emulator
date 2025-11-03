%{
#include <cstdio>
#include <cstdlib>
#include <string>
#include "../../../build/assembler/parser/parser.tab.hpp"

int yylex();
%}

%%

[ \t\r\n]+                ;
\#.*                      ;

\.global                   { printf("recognized token: DIR_GLOBAL\n"); return DIR_GLOBAL; }
\.extern                   { printf("recognized token: DIR_EXTERN\n"); return DIR_EXTERN; }
\.section                  { printf("recognized token: DIR_SECTION\n"); return DIR_SECTION; }
\.word                     { printf("recognized token: DIR_WORD\n"); return DIR_WORD; }
\.skip                     { printf("recognized token: DIR_SKIP\n"); return DIR_SKIP; }
\.ascii                    { printf("recognized token: DIR_ASCII\n"); return DIR_ASCII; }
\.equ                      { printf("recognized token: DIR_EQU\n"); return DIR_EQU; }
\.end                      { printf("recognized token: DIR_END\n"); return DIR_END; }

halt                       { printf("recognized token: HALT\n"); return HALT; }
int                        { printf("recognized token: INT\n"); return INT; }
iret                       { printf("recognized token: IRET\n"); return IRET; }
call                       { printf("recognized token: CALL\n"); return CALL; }
ret                        { printf("recognized token: RET\n"); return RET; }
jmp                        { printf("recognized token: JMP\n"); return JMP; }
beq                        { printf("recognized token: BEQ\n"); return BEQ; }
bne                        { printf("recognized token: BNE\n"); return BNE; }
bgt                        { printf("recognized token: BGT\n"); return BGT; }
push                       { printf("recognized token: PUSH\n"); return PUSH; }
pop                        { printf("recognized token: POP\n"); return POP; }
xchg                       { printf("recognized token: XCHG\n"); return XCHG; }
add                        { printf("recognized token: ADD\n"); return ADD; }
sub                        { printf("recognized token: SUB\n"); return SUB; }
mul                        { printf("recognized token: MUL\n"); return MUL; }
div                        { printf("recognized token: DIV\n"); return DIV; }
not                        { printf("recognized token: NOT\n"); return NOT; }
and                        { printf("recognized token: AND\n"); return AND; }
or                         { printf("recognized token: OR\n"); return OR; }
xor                        { printf("recognized token: XOR\n"); return XOR; }
shl                        { printf("recognized token: SHL\n"); return SHL; }
shr                        { printf("recognized token: SHR\n"); return SHR; }
ld                         { printf("recognized token: LD\n"); return LD; }
st                         { printf("recognized token: ST\n"); return ST; }
csrrd                      { printf("recognized token: CSRRD\n"); return CSRRD; }
csrwr                      { printf("recognized token: CSRWR\n"); return CSRWR; }

%r([0-9]|1[0-5])           { yylval.int_val = atoi(yytext + 2); printf("recognized token: GPR (%s)\n", yytext); return GPR; }
%sp                        { yylval.int_val = 14; printf("recognized token: GPR (%s)\n", yytext); return GPR; }
%pc                        { yylval.int_val = 15; printf("recognized token: GPR (%s)\n", yytext); return GPR; }

%cause                      { yylval.int_val = 2; printf("recognized token: CSR (%s)\n", yytext); return CSR; }
%handler                    { yylval.int_val = 1; printf("recognized token: CSR (%s)\n", yytext); return CSR; }
%status                     { yylval.int_val = 0; printf("recognized token: CSR (%s)\n", yytext); return CSR; }

\$                          { return DOLAR; }
%                           { return PERCENT; }
,                           { return COMMA; }
:                           { return COLON; }
\(                          { return LBRACKET; }
\)                          { return RBRACKET; }
\[                          { return LBRACKET2; }
\]                          { return RBRACKET2; }
\+                          { return PLUS; }
-                           { return MINUS; }
\*                          { return STAR; }
\/                          { return SLASH; }

0[xX][0-9A-Fa-f]+           { yylval.int_val = strtol(yytext, nullptr, 16); printf("recognized token: HEX_NUMBER (%s)\n", yytext); return HEX_NUMBER; }
[0-9]+                       { yylval.int_val = atoi(yytext); printf("recognized token: TOKEN_NUMBER (%s)\n", yytext); return TOKEN_NUMBER; }

[A-Za-z_][A-Za-z0-9_]*       { yylval.str_val = new std::string(yytext); printf("recognized token: TOKEN_ID (%s)\n", yytext); return TOKEN_ID; }
\"([^\\\"]|\\.)*\"          {
                              std::string raw(yytext);
                              // Remove the surrounding quotes
                              std::string content = raw.substr(1, raw.size() - 2);
                              yylval.str_val = new std::string(content);
                              printf("recognized token: TOKEN_STRING (%s)\n", yytext);
                              return TOKEN_STRING;
                            }

.                            { printf("recognized token: TOKEN_UNKNOWN (%s)\n", yytext); return -1; }


%%

int yywrap() { return 1; }
