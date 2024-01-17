%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "calc.h"
    FILE *logFile;
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

%left ADD SUB
%left MUL DIV
%nonassoc <fn> CMP
%right EQ
%left EOL


%type <a> exp stmt list explist assignment cast_expression
%type <s> declaration
%type <sl> symlist

%start S
%%
S: 
    | S list EOL 
    {
        fprintf(logFile, "= %4.4g\n", eval($2));
        treefree($2);
        printf("> ");
    }
    | S LET NAME OP symlist CP EQ list EOL 
    { 
        dodef($3, $5, $8); 
        fprintf(logFile, "defined %s\n> ", $3->name);
    }
    | S error EOL { yyerrok; printf("> "); }
    ;

exp:  exp CMP exp { fprintf(logFile, "entered cmp \n"); $$ = newcmp($2, $1, $3); }
    | exp ADD exp { fprintf(logFile, "entered add \n"); $$ = newast('+', $1, $3); }
    | exp SUB exp { fprintf(logFile, "entered sub \n"); $$ = newast('-', $1, $3); }
    | exp MUL exp { fprintf(logFile, "entered mul \n"); $$ = newast('*', $1, $3); }
    | exp DIV exp { fprintf(logFile, "entered div \n"); $$ = newast('/', $1, $3); }
    | OP exp CP   { fprintf(logFile, "entered brackets\n"); $$ = $2; }
    | NUMBER      { fprintf(logFile, "entered number \n"); $$ = newnum($1); }
    | NAME        { fprintf(logFile, "entered name \n"); $$ = newref(lookup($1, 0)); }
    | FUNC OP explist CP { fprintf(logFile, "entered function\n"); $$ = newfunc($1, $3); }    
    | NAME OP explist CP { $$ = newcall($1, $3); }
    | cast_expression { $$ = $1; }
    ;

explist: exp
    | exp COMMA explist { fprintf(logFile, "entered expression list\n"); $$ = newast('L', $1, $3); }
    ;

declaration: TYPE NAME { fprintf(logFile, "entered type name\n"); $$ = lookup($2, $1); }
    ;

symlist: declaration { $$ = newsymlist($1, NULL); }
    | declaration COMMA symlist  { $$ = newsymlist($1, $3); }
    ;

assignment: NAME EQ exp
    { 
        fprintf(logFile, "entered name eq exp\n");
        $$ = newasgn(lookup($1, 0), $3);
    }    
    | declaration EQ exp 
    { 
        fprintf(logFile, "entered type name eq exp\n");
        $$ = newasgn($1, $3);
    }
    ;
    
cast_expression: OP TYPE CP exp %prec CMP
        { $$ = newcast($2, $4); }
	;

stmt: IF OP exp CP OB list CB { fprintf(logFile, "entered if\n"); $$ = newflow('I', $3, $6, NULL); }
    | IF OP exp CP OB list CB ELSE OB list CB { fprintf(logFile, "entered if else\n"); $$ = newflow('I', $3, $6, $10); }
    | WHILE OP exp CP OB list CB { fprintf(logFile, "entered while\n"); $$ = newflow('W', $3, $6, NULL); }
    | exp { fprintf(logFile, "entered stmt exp\n"); }
    | assignment { fprintf(logFile, "entered stmt assignment\n"); $$ = $1; }
    | declaration { fprintf(logFile, "entered stmt declaration\n"); $$ = newref($1); }
    ;

list:               { fprintf(logFile, "entered list empty \n"); $$ = NULL; }
    | stmt SEP list { fprintf(logFile, "entered list stmt \n"); if ($3 == NULL) $$ = $1; else $$ = newast('L', $1, $3); }
    ;

%%

int main() {
    logFile = fopen("calc.log", "w");
    printf("> ");
    setbuf(logFile, NULL);
    yyparse();
    fclose(logFile);
    return 0;
}
