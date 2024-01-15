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
%token OP CP OB CB

%right EQ
%left ADD SUB
%left MUL DIV
%nonassoc <fn> CMP

%left EOL

%type <a> exp stmt list explist assignment
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

exp:  exp CMP exp { printf("entered cmp rule\n"); $$ = newcmp($2, $1, $3); }
    | exp ADD exp { printf("entered add rule\n"); $$ = newast('+', $1, $3); }
    | exp SUB exp { printf("entered sub rule\n"); $$ = newast('-', $1, $3); }
    | exp MUL exp { printf("entered mul rule\n"); $$ = newast('*', $1, $3); }
    | exp DIV exp { printf("entered div rule\n"); $$ = newast('/', $1, $3); }
    | OP exp CP   { printf("entered brackets\n"); $$ = $2; }
    | NUMBER      { printf("entered number rule\n"); $$ = newnum($1); }
    | NAME 
    {    
        printf("entered name rule\n");
        $$ = newref(lookup($1, 0)); 
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
    | assignment {printf("entered assignment rule\n");}
    ;

assignment: NAME EQ exp
    { 
        printf("entered name eq exp\n");
        $$ = newasgn(lookup($1, 0), $3);
        if ($$ == NULL) {
            printf("Error: Variable is undeclared.\n");
            YYERROR;
        }   
    }    
    | TYPE NAME EQ exp 
    { 
        printf("entered type name eq exp\n");
        $$ = newasgn(lookup($2, $1), $4);
        if ($$ == NULL) {
            printf("Error: Variable is already declared.\n");
            YYERROR;
        }  
    }
    ;

cast: TYPE OP exp CP { 
                        printf("new cast\n");
                        $$ = newcast($1, $3);
                    }


explist: exp
    | exp COMMA explist { printf("entered expression list\n"); $$ = newast('L', $1, $3); }
    ;

symlist: NAME { $$ = newsymlist($1, NULL); }
    | NAME COMMA symlist  { $$ = newsymlist($1, $3); }
    ;

stmt: IF OP exp CP OB list CB { printf("entered if\n"); $$ = newflow('I', $3, $6, NULL); }
    | IF OP exp CP OB list CB ELSE OB list CB { printf("entered if else\n"); $$ = newflow('I', $3, $6, $10); }
    | WHILE OP exp CP OB list CB { printf("entered while\n"); $$ = newflow('W', $3, $6, NULL); }
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
