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
    type t;
}

%token <t> TYPE
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
    | S list EOL 
    {
        printf("entered start\n");
        printf("= %4.4g\n", eval($2));
        treefree($2);
        printf("> ");
    }
    | S LET NAME OP symlist CP EQ list EOL 
    { 
        dodef($3, $5, $8); 
        printf("defined %s\n> ", $3->name);
    }
    | S error EOL { yyerrok; printf("> "); }
    ;

exp:  exp CMP exp { $$ = newcmp($2, $1, $3); }
    | exp ADD exp { $$ = newast('+', $1, $3); }
    | exp SUB exp { $$ = newast('-', $1, $3); }
    | exp MUL exp { $$ = newast('*', $1, $3); }
    | exp DIV exp { $$ = newast('/', $1, $3); }
    | OP exp CP { $$ = $2; }
    | NUMBER { $$ = newnum($1); }
    | NAME 
    {    
        $$ = newref(lookup($1, 0)); 
        if ($$ == NULL) {
            printf("Error: Variable is undeclared.\n");
            YYERROR;
        }
    }
    | NAME EQ exp 
    { 
        printf("entered name eq exp\n");
        $$ = newasgn(lookup($1, 0), $3);
        if ($$ == NULL) {
            printf("Error: Variable is undeclared.\n");
            YYERROR;
        }   
    }    
    | FUNC OP explist CP { $$ = newfunc($1, $3); }    
    | NAME OP explist CP { $$ = newcall($1, $3); } 
    | TYPE NAME 
    {   
        $$ = newref(lookup($2, $1)); 
        if ($$ == NULL) {
            printf("Error: Variable is already declared.\n");
            YYERROR;
        }  
    }
    | TYPE NAME EQ exp 
    { 
        $$ = newasgn(lookup($2, $1), $4);
        if ($$ == NULL) {
            printf("Error: Variable is already declared.\n");
            YYERROR;
        }  
    }
    ;

explist: exp
    | exp COMMA explist { printf("entered expression list\n"); $$ = newast('L', $1, $3); }
    ;

symlist: NAME { $$ = newsymlist($1, NULL); }
    | NAME COMMA symlist  { $$ = newsymlist($1, $3); }
    ;

stmt: IF exp THEN list { $$ = newflow('I', $2, $4, NULL); }
    | IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
    | WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
    | exp { printf("entered stmt exp rule\n"); }
    ;

list:               { printf("entered list empty rule\n"); $$ = NULL; }
    | stmt SEP list { printf("entered list stmt rule\n"); if ($3 == NULL) $$ = $1; else $$ = newast('L', $1, $3); }
    ;

%%

int main() {

    printf("> ");
    yyparse();

    return 0;
}
