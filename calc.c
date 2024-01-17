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

ast *newast(int nodetype, ast *l, ast *r)
{
    printf("entered new ast\n");
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
    printf("function eval: %lf\n", v);
    switch(functype) {
        case B_print:
            printf("= %4.4g\n", v);
            return v;
        case B_scanf:
            printf("scanf: ");
            double newval;
            scanf("%lf", &newval); 
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            ((symref *)f->l)->s->value = newval;
            printf("new value of variable: %lf\n", ((symref *)f->l)->s->value);
            return newval;
        default:
            yyerror("unknown function %d", functype);
            return 0;
    }
}

ast *newcast(type t, ast *exp) {
    cast *c = malloc(sizeof(cast));
    if (!c) {
        yyerror("malloc ast");
        exit(-10);
    }
    (c)->nodetype = 'T';
    c->t = t;
    c->v = exp;
    return (ast*)c;
}

double eval(ast *a) {
    printf("eval\n");
    printf("nodetype: %c\n", a->nodetype);
    double result;

    switch(a->nodetype) {
        //constant
        case 'K':
            printf("eval constant\n");
            result = ((numval*)a)->number;
            printf("result : %lf\n", result);
            break;
        case 'N':
            printf("eval symbol ref\n");
            result = ((symref*)a)->s->value; 
            printf("result : %lf\n", result);
            break;
        case '=':
            printf("eval =\n");
            symasgn *tmp = (symasgn*)a;
            result = eval(tmp->v);
            printf("%i\n", tmp->s->t);
            switch(tmp->s->t) {
                case T_INT:
                    printf("eval int\n");
                    tmp->s->value = (int)result;
                    result = tmp->s->value;
                    break;
                case T_FLOAT:
                    printf("eval float\n");
                    tmp->s->value = (float)result;
                    result = tmp->s->value;
                    break;
                case T_DOUBLE:
                    printf("eval double\n");
                    tmp->s->value = (double)result;
                    result = tmp->s->value;
                    break;
                default:
                    yyerror("unknown type %i", (tmp->s->t));
                    break;
            } 
            printf("result : %lf\n", result);
            break;
        case '+':
            result = eval(a->l) + eval(a->r);
            printf("result : %lf\n", result);
            break;
        case '-':
            result = eval(a->l) - eval(a->r);
            printf("result : %lf\n", result);
            break;
        case '*':
            result = eval(a->l) * eval(a->r);
            printf("result : %lf\n", result);
            break;
        case '/':
            result = eval(a->l) / eval(a->r);
            printf("result : %lf\n", result);
            break;
        case 'M':
            result = -eval(a->l);
            break;
        case '1': result = (eval(a->l) > eval(a->r)) ? 1 : 0; break;
        case '2': result = (eval(a->l) < eval(a->r)) ? 1 : 0; break;
        case '3': result = (eval(a->l) != eval(a->r)) ? 1 : 0; break;
        case '4': 
            result = (eval(a->l) == eval(a->r)) ? 1 : 0; 
            break;
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
        case 'L': printf("L\n"); eval(a->l); result = eval(a->r); break;
        case 'F': printf("F\n"); result = callbuiltin((fncall*)a); break;
        case 'C': printf("C\n"); result = calluser((ufncall*)a); break;
        case 'T':
            printf("entered typecast\n");
            result = eval(((cast*)a)->v);
            type targetType = ((cast*)a)->t;
            switch (targetType) {
                case T_INT:
                    result = (int)result;
                    break;
                case T_FLOAT:
                    result = (float)result;
                    break;
                case T_DOUBLE:
                    result = (double)result;
                    break;
            }
            break;
        default:
            printf("eval bad node %c\n", a->nodetype);
            break;   
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
            printf("free L\n");
            treefree(a->r);
            break;
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
            break;
        case 'T':
            free(((cast*)a)->v);
            break;
        default: 
            printf("tree free bad node %c\n", a->nodetype);
            break;       
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

symbol *lookup(char *sym, type t) {
    printf("entered lookup \n");
    printf("sym: %s type %i\n", sym, t);
    symbol *sp = &symtab[symhash(sym)%NHASH];
    int scount = NHASH;
    while(--scount > 0) {
        if(sp->name && !strcmp(sp->name, sym)) { 
            if(t == 0) {
                return sp;
            }
            if(sp->t != t) {
                yyerror("conflicting types for %s", sym);
                return NULL;
            }
            if(sp->t == t) {
                yyerror("%s already declared", sym);
                return NULL;
            }
            return sp; 
        }
        if(!sp->name) {
            if(t == 0) {
                yyerror("undefined symbol type %s", sym);
                return NULL;
            }
            sp->name = strdup(sym);
            sp->value = 0;
            sp->t = t;
            sp->func = NULL;
            sp->syms = NULL;
            printf("new symbol %s of type %i\n", sym, t);
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
    printf("assigned ok %lf\n", eval(v));
    return (ast*)a;
}

ast *newflow(int nodetype, ast* cond, ast *tl, ast *el) {
    printf("entered newflow\n");
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
    for(nargs = 0; sl; sl = sl->next) {
        nargs++;
    
    }
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