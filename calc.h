#ifndef CALC_H
#define CALC_H
#include <string.h>

#define NHASH 9997

extern int yylineno;
void yyerror(char*s, ...);

enum type {
    T_INT = 1,
    T_FLOAT = 2,
    T_DOUBLE = 3
};

typedef enum type type;

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
    type t;
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
    B_print = 1,
    B_scanf = 2
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

typedef struct cast {
    int nodetype;
    type t;
    ast *v;
} cast;

typedef struct scope {
    symbol *symtab;
    struct scope *parent;
} scope;

extern scope *topscope;
extern FILE *logFile;
extern FILE* yyin;

typedef struct func {
    symlist *params;
    ast *body;
} func;

enum type get_type(double d);
ast *newast(int nodetype, ast *l, ast *r);
ast *newnum(double d);
double eval(ast*);
void treefree(ast*);
symlist *newsymlist(symbol *sym, symlist* next);
void symlistfree(symlist *sl);
symbol *lookup(char*, type);
ast *newcmp(int cmptype, ast* l, ast *r);
ast *newfunc(int functype, ast *l);
ast *newcall(symbol *s, ast* l);
ast *newref(symbol *s);
ast *newasgn(symbol *s, ast *v);
ast *newflow(int nodetype, ast *cond, ast *tl, ast *tr);
double callbuiltin(fncall *);
double calluser(ufncall *);
void dodef(symbol *name, symlist *syms, ast *stmts);
ast *newcast(type t, ast *v);
scope* new_scope();
void push_scope();
void pop_scope(); 

#endif // CALC_H