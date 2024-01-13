#include <string.h>

extern int yylineno;
void yyerror(char*s, ...);

typedef struct ast {
    int nodetype;
    struct ast* l;
    struct ast *r;
} ast;

typedef struct numval {
    int nodetype;
    double number;
} numval;

typedef struct symbol {
    char *name;
    double value;
    ast *func;
    struct symlist *syms;
} symbol;

typedef struct symlist {
    symbol *sym;
    struct symlist *next;
} symlist;

enum bifs { /* built-in functions */
    B_print = 1
};

typedef struct fncall {
    int nodetype;
    ast *l;
    enum bifs functype;
} fncall;

typedef struct ufncall {
    int nodetype;
    ast *l;
    symbol *s;
} ufncall;

typedef struct flow {
    int nodetype;
    ast *cond;
    ast *tl;
    ast *el;
} flow;

typedef struct symref {
    int nodetype;
    symbol *s;
} symref;

typedef struct symasgn {
    int nodetype;
    symbol *s;
    ast *v;
} symasgn;

ast *newast(int nodetype, ast *l, ast *r);
ast *newnum(double d);
double eval(ast*);
void treefree(ast*);
symlist *newsymlist(symbol *sym, symlist* next);
void symlistfree(symlist *sl);
symbol *lookup(char*);
ast *newcmp(int cmptype, ast* l, ast *r);
ast *newfunc(int functype, ast *l);
ast *newcall(symbol *s, ast* l);
ast *newref(symbol *s);
ast *newasgn(symbol *s, ast *v);
ast *newflow(int nodetype, ast *cond, ast *tl, ast *tr);
double callbuiltin(fncall *);
double calluser(ufncall *);
void dodef(symbol *name, symlist *syms, ast *stmts);