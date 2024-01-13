%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "calc.h"
%}


%union {
    struct ast *a;
    double d;
    struct symbol *s;
    struct symlist *sl;
    int fn;
}

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token IF THEN ELSE WHILE DO LET
%token EOL SEP COMMA
%token ADD SUB MUL DIV
%token OP CP

%left ADD SUB
%left MUL DIV

%nonassoc <fn> CMP
%right EQ

%type <a> exp stmt list explist
%type <sl> symlist

%start S
%%
S: 
    | S stmt EOL {
                    printf("entered start\n");
                    printf("= %4.4g\n", eval($2));
                    treefree($2);
                }
    | S LET NAME OP symlist CP EQ list EOL { dodef($3, $5, $8); printf("defined %s\n> ", $3->name); }
    | S error EOL { yyerrok; printf("> "); }
    ;

exp: exp CMP exp { $$ = newcmp($2, $1, $3); }
    | exp ADD exp { $$ = newast('+', $1, $3); }
    | exp SUB exp { $$ = newast('-', $1, $3); }
    | exp MUL exp { $$ = newast('*', $1, $3); }
    | exp DIV exp { $$ = newast('/', $1, $3); }
    | OP exp CP { $$ = $2; }
    | NUMBER { printf("entered NUMBER\n"); $$ = newnum($1); }
    | NAME { printf("entered NAME\n"); $$ = newref($1); }
    | NAME EQ exp { $$ = newasgn($1, $3); }    
    | FUNC OP explist CP { $$ = newfunc($1, $3); }    
    | NAME OP explist CP { $$ = newcall($1, $3); }    
    ;

explist: exp
    | exp ',' explist { $$ = newast('L', $1, $3); }
    ;

symlist: NAME { $$ = newsymlist($1, NULL); }
    | NAME ',' symlist  { $$ = newsymlist($1, $3); }
    ;

stmt: IF exp THEN list { $$ = newflow('I', $2, $4, NULL); }
    | IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
    | WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
    | exp
    ;

list: {$$ = NULL;}
    | stmt ';' list { if ($3 == NULL) $$ = $1; else $$ = newast('L', $1, $3); }
    ;
%%