#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "calc.h"

#define NHASH 9997
symbol symtab[NHASH];

/* node types
 * + - * / |
 * 0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 * M unary minus
 * L expression or statement list
 * I IF statement
 * W WHILE statement
 * N symbol ref
 * = assignment
 * S list of symbols
 * F built in function call
 * C user function call
 */ 
ast *newast(int nodetype, ast *l, ast *r) {
    ast *a = malloc(sizeof(ast));
    if(!a) {
        yyerror("ast alloc");
        exit(1);
    }

    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

ast* newnum(double d) {
    numval *n = malloc(sizeof(numval));
    if(!n) {
        yyerror("newnum alloc");
        exit(2);
    }
    n->nodetype = 'K';
    n->number = d;
    return (ast*) n;
}

double callbuiltin(fncall *f) {
    enum bifs functype = f->functype;
    double v = eval(f->l);

    switch(functype) {
        case B_print:
            printf("= %4.4g\n", v);
            return v;
        default:
            yyerror("unknown function %d", functype);
            return 0;
    }
}

double eval(ast *a) {
    double result;

    if(!a) {
        yyerror("null ast");
        return 0;
    }

    switch(a->nodetype) {
        //constant
        case 'K': 
            printf("entered constant\n");
            result = ((numval*)a)->number; 
            break;
        case 'N':
            printf("entered symbol ref\n");
            result = ((symref*)a)->s->value; 
            break;
        case '=':
            printf("entered =\n");
            symasgn *tmp = (symasgn*)a;
            result = eval(tmp->v);
            tmp->s->value = result;
            break;
        case '+':
            result = eval(a->l) + eval(a->r);
            break;
        case '-':
            result = eval(a->l) - eval(a->r);
            break;
        case '*':
            result = eval(a->l) * eval(a->r);
            break;
        case '/':
            result = eval(a->l) / eval(a->r);
            break;
        case 'M':
            result = -eval(a->l);
            break;
        case '1': result = (eval(a->l) > eval(a->r)) ? 1 : 0; break;
        case '2': result = (eval(a->l) < eval(a->r)) ? 1 : 0; break;
        case '3': result = (eval(a->l) != eval(a->r)) ? 1 : 0; break;
        case '4': result = (eval(a->l) == eval(a->r)) ? 1 : 0; break;
        case '5': result = (eval(a->l) >= eval(a->r)) ? 1 : 0; break;
        case '6': result = (eval(a->l) <= eval(a->r)) ? 1 : 0; break;

        case 'I':
            if(eval(((flow*)a)->cond) != 0) {
                if(((flow*)a)->tl) {
                    result = eval(((flow*)a)->tl);
                }
                else result = 0;
            }
            else 
            {
                if(((flow*)a)->el) {
                    result = eval(((flow*)a)->el);
                }
                else result = 0;
            }
            break;
        case 'W':
            result = 0;
            if(((flow*)a)->tl) {
                while(eval(((flow*)a)->cond) != 0)
                    result = eval(((flow*)a)->tl);
            }
            break;
        case 'L': eval(a->l); result = eval(a->r); break;
        case 'F': result = callbuiltin((fncall*)a); break;
        case 'C': result = calluser((ufncall*)a); break;
        default:
            printf("bad node %c\n", a->nodetype);   
    }
    return result; 
}

void treefree(ast* a) {
    switch(a->nodetype) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '1': case '2': case '3': case '4': case '5':
        case 'L':
            treefree(a->r);
        case 'M': case 'C' : case 'F':
            treefree(a->l);
        case 'K': case 'N':
            break;
        case '=':
            free(((symasgn*)a)->v);
            break;
        case 'I': case 'W':
            free(((flow*)a)->cond);
            if(((flow*)a)->tl) treefree(((flow*)a)->tl);
            if(((flow*)a)->el) treefree(((flow*)a)->el);
        default: printf("bad node %c\n", a->nodetype);       
    }

    free(a);
}

static unsigned symhash(char * sym) {
    unsigned int hash = 0;
    unsigned c;
    while(c = *sym++) {
        hash = hash * 9 ^ c;
    }
    return hash;
}

symbol *lookup(char *sym) {
    symbol *sp = &symtab[symhash(sym)%NHASH];
    int scount = NHASH;
    while(--scount > 0) {
        if(sp->name && !strcmp(sp->name, sym)) { return sp; }
        if(!sp->name) {
            sp->name = strdup(sym);
            printf("name %s assigned\n", sp->name);
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }
        if(++sp >= symtab+NHASH) sp = symtab; 
    }
    yyerror("symbol table overflow\n");
    abort();
}

ast* newcmp(int cmptype, ast *l, ast *r) {
    ast *a = malloc (sizeof(ast));
    if(!a) {
        yyerror("malloc ast");
        exit(-3);
    }

    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

ast *newfunc(int functype, ast* l) {
    fncall *a = malloc(sizeof(fncall));
    if(!a) {
        yyerror("malloc ast");
        exit(-4);
    }
    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (ast*) a;
}

ast *newcall(symbol *s, ast *l) {
    ufncall *a = malloc(sizeof(ufncall));
    if(!a) {
        yyerror("malloc ast");
        exit(-5);
    }
    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    return (ast*) a;
}

ast *newref(symbol *s) {
    symref *a = malloc(sizeof(symref));
    if(!a) {
        yyerror("malloc ast");
        exit(-6);
    }
    a->nodetype = 'N';
    a->s = s;
    return (ast*) a;
}

ast *newasgn(symbol *s, ast *v) {
    symasgn *a = malloc(sizeof(symasgn));
    if(!a) {
        yyerror("malloc ast");
        exit(-7);
    }

    a->nodetype = '=';
    a->s = s;
    a->v = v;
    printf("new asgn assigned ok\n");
    return (ast*)a;
}

ast *newflow(int nodetype, ast* cond, ast *tl, ast *el) {
    flow *a = malloc(sizeof(flow));
    if(!a) {
        yyerror("malloc ast");
        exit(-8);
    }

    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (ast*) a;
}

symlist* newsymlist(symbol *sym, symlist *next) {
    symlist *sl = malloc(sizeof(symlist));
    if(!sl) {
        yyerror("malloc ast");
        exit(-9); 
    }
    sl->sym = sym;
    sl->next = next;
    return sl;
}

void symlistfree(symlist *sl) {
    symlist* nsl;
    while(sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

double calluser (ufncall *f) {
    symbol *fn = f->s;
    symlist *sl;
    ast *args = f->l;
    double  *oldval, *newval;
    double v;
    int nargs;
    int i;

    if(!fn->func) {
        yyerror("call to undefined function", fn->name);
        return 0;
    }

    sl = fn->syms;
    for(nargs = 0; sl; sl = sl->next)
        nargs++;
    oldval = (double*) malloc(nargs*sizeof(double));
    newval = (double*) malloc(nargs*sizeof(double));

    if(!oldval || !newval) {
        yyerror("malloc");
        return 0;
    }

    for(i = 0; i < nargs; i++) {
        if(!args){
            yyerror("too few args in call to %s", fn->name);
            free(oldval);
            free(newval);
            return 0;
        }

        if(args->nodetype == 'L') {
            newval[i] = eval(args->l);
            args = args->r;
        }
        else 
        {
            newval[i] = eval(args);
            args = NULL;
        }
    }

    sl = fn->syms;

    for(int i = 0; i < nargs; i++) {
        symbol *s = sl->sym;
        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }

    free(newval);

    v = eval(fn->func);

    sl = fn->syms;
    for(i = 0; i < nargs; i++) {
        symbol *s = sl->sym;
        s->value = oldval[i];
        sl = sl->next;
    }
    free(oldval);
    return v;
}
    
void dodef(symbol *name, symlist *syms, ast *func) {
    if(name->syms) symlistfree(name->syms);
    if(name->func) treefree(name->func);
    name->syms = syms;
    name->func = func;
}

void yyerror(char *s, ...) {
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int main(){
    printf("> ");
    return yyparse();
}