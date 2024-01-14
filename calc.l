%{
    #include "calc.h"
    #include "calc.tab.h"
%}

EXP ([Ee][-+]?[0-9]+)

%%
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"(" { return OP; }
")" { return CP; }
"=" { return EQ; }
"," { return COMMA; }
";" { return SEP; }

">" { yylval.fn = 1; return CMP; }
"<" { yylval.fn = 2; return CMP; }
"!=" { yylval.fn = 3; return CMP; }
"==" { yylval.fn = 4; return CMP; }
">=" { yylval.fn = 5; return CMP; }
"<=" { yylval.fn = 6; return CMP; }

"int" { yylval.t = T_INT; return TYPE; }
"float" { yylval.t = T_FLOAT; return TYPE; }
"double" { yylval.t = T_DOUBLE; return TYPE; }

"if" { return IF; }
"then" { return THEN; }
"else" { return ELSE; }
"while" { return WHILE; }
"do" { return DO; }
"let" { return LET; }

"print" { yylval.fn = B_print; return FUNC; }

 /* names */
[a-zA-Z][a-zA-Z0-9]* { printf("%d\n", yylval.t);
    yylval.s = lookup(yytext, yylval.t); return NAME; }

 /* values */
[0-9]+"."[0-9]*{EXP}?|"."?[0-9]+{EXP}?"f"? { yylval.d = atof(yytext); return NUMBER; }
                                        
 /* comments and whitespace */
"//".* { /* ignore comments */ }
[ \t]* { /* ignore whitespace */ }

\\\n   {    }
\n { return EOL; }
. { printf("Mystery character %c\n", *yytext); }
%%
