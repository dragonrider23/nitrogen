#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "ncore.h"

void nenv_add_builtin(nenv* e, char* name, nbuiltin func) {
    nval* k = nval_sym(name);
    nval* v = nval_fun(func);
    nenv_put(e, k, v);
    nval_del(k);
    nval_del(v);
}

void nenv_add_builtins(nenv* e) {
    /* Variables and functions */
    nenv_add_builtin(e, "def", builtin_def);
    nenv_add_builtin(e, "undef", builtin_undef);
    nenv_add_builtin(e, "\\", builtin_lambda);
    nenv_add_builtin(e, "=", builtin_put);

    /* List Functions */
    nenv_add_builtin(e, "list", builtin_list);
    nenv_add_builtin(e, "head", builtin_head);
    nenv_add_builtin(e, "tail", builtin_tail);
    nenv_add_builtin(e, "eval", builtin_eval);
    nenv_add_builtin(e, "join", builtin_join);

    /* Mathematical Functions */
    nenv_add_builtin(e, "+", builtin_add);
    nenv_add_builtin(e, "-", builtin_sub);
    nenv_add_builtin(e, "*", builtin_mul);
    nenv_add_builtin(e, "/", builtin_div);

    /* Logical operators */
    nenv_add_builtin(e, "if", builtin_if);
    nenv_add_builtin(e, "==", builtin_eq);
    nenv_add_builtin(e, "!=", builtin_ne);
    nenv_add_builtin(e, ">",  builtin_gt);
    nenv_add_builtin(e, "<",  builtin_lt);
    nenv_add_builtin(e, ">=", builtin_ge);
    nenv_add_builtin(e, "<=", builtin_le);
}

/* Builtin arithmatic operations */
nval* builtin_op(nenv* e, nval* a, char* op) {
    for (int i = 0; i < a->count; i++) {
        LASSERT_TYPE(op, a, i, NVAL_NUM);
    }

    nval* x = nval_pop(a, 0);

    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    while (a->count > 0) {
        nval* y = nval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                nval_del(x);
                nval_del(y);
                x = nval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }
        if (strcmp(op, "%") == 0) {
            if (y->num == 0) {
                nval_del(x);
                nval_del(y);
                x = nval_err("Division By Zero!"); break;
            }
            x->num %= y->num;
        }

        nval_del(y);
    }

    nval_del(a);
    return x;
}

nval* builtin_add(nenv* e, nval* a) {
    return builtin_op(e, a, "+");
}

nval* builtin_sub(nenv* e, nval* a) {
    return builtin_op(e, a, "-");
}

nval* builtin_mul(nenv* e, nval* a) {
    return builtin_op(e, a, "*");
}

nval* builtin_div(nenv* e, nval* a) {
    return builtin_op(e, a, "/");
}

/* Take a Q-Expression and return a Q-Expression with only the first element */
nval* builtin_head(nenv* e, nval* a) {
    LASSERT_NUM("head", a, 1);
    LASSERT_TYPE("head", a, 0, NVAL_QEXPR);
    LASSERT_NOT_EMPTY("head", a, 0);

    nval* v = nval_take(a, 0);
    while (v->count > 1) {
        nval_del(nval_pop(v, 1));
    }
    return v;
}

/* Take a Q-Expression and return a Q-Expression with the first element removed */
nval* builtin_tail(nenv* e, nval* a) {
    LASSERT_NUM("tail", a, 1);
    LASSERT_TYPE("tail", a, 0, NVAL_QEXPR);
    LASSERT_NOT_EMPTY("tail", a, 0);

    nval* v = nval_take(a, 0);
    nval_del(nval_pop(v, 0));
    return v;
}

/* Convert S-expression to Q-expression */
nval* builtin_list(nenv* e, nval* a) {
    a->type = NVAL_QEXPR;
    return a;
}

/* Evaluate Q-expression like S-express */
nval* builtin_eval(nenv* e, nval* a) {
    LASSERT_NUM("eval", a, 1);
    LASSERT_TYPE("eval", a, 0, NVAL_QEXPR);

    nval* x = nval_take(a, 0);
    x->type = NVAL_SEXPR;
    return nval_eval(e, x);
}

/* Join multiple Q-expressions into one */
nval* builtin_join(nenv* e, nval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == NVAL_QEXPR,
            "Function 'join' was passed incorrect type");
    }

    nval* x = nval_pop(a, 0);
    while (a->count) {
        x = nval_join(x, nval_pop(a, 0));
    }
    nval_del(a);
    return x;
}

nval* builtin_def(nenv* e, nval* a) {
    return builtin_var(e, a, "def");
}

nval* builtin_put(nenv* e, nval* a) {
    return builtin_var(e, a, "=");
}

nval* builtin_var(nenv* e, nval* a, char* func) {
    LASSERT_TYPE(func, a, 0, NVAL_QEXPR);

    nval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, (syms->cell[i]->type == NVAL_SYM),
          "Function '%s' cannot define non-symbol. "
          "Got %s, Expected %s.", func, 
          ntype_name(syms->cell[i]->type),
          ntype_name(NVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count-1),
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.", func, syms->count, a->count-1);

    for (int i = 0; i < syms->count; i++) {
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            nenv_def(e, syms->cell[i], a->cell[i+1]);
        }

        if (strcmp(func, "=")   == 0) {
            nenv_put(e, syms->cell[i], a->cell[i+1]);
        } 
    }

    nval_del(a);
    return nval_sexpr();
}

nval* builtin_undef(nenv* e, nval* a) {
    LASSERT(a, a->cell[0]->type == NVAL_QEXPR,
        "Function 'undef' passed incorrect type");

    nval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == NVAL_SYM,
            "Function 'undef' cannot define non-symbol");
    }

    LASSERT_NUM("undef", a, 1);

    for (int i = 0; i < syms->count; i++) {
        nenv_rem(e, syms->cell[i]);
    }
    nval_del(a);
    return nval_sexpr();
}

nval* builtin_lambda(nenv* e, nval* a) {
    LASSERT_NUM("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, NVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, NVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == NVAL_SYM),
            ntype_name(a->cell[0]->cell[i]->type), ntype_name(NVAL_SYM));
    }

    nval* formals = nval_pop(a, 0);
    nval* body = nval_pop(a, 0);
    nval_del(a);
    return nval_lambda(formals, body);
}

nval* builtin_gt(nenv* e, nval* a) {
    return builtin_ord(e, a, ">");
}

nval* builtin_lt(nenv* e, nval* a) {
    return builtin_ord(e, a, "<");
}

nval* builtin_ge(nenv* e, nval* a) {
    return builtin_ord(e, a, ">=");
}

nval* builtin_le(nenv* e, nval* a) {
    return builtin_ord(e, a, "<=");
}

nval* builtin_ord(nenv* e, nval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    LASSERT_TYPE(op, a, 0, NVAL_NUM);
    LASSERT_TYPE(op, a, 1, NVAL_NUM);

    int r;
    if (strcmp(op, ">") == 0) {
        r = (a->cell[0]->num > a->cell[1]->num);
    }
    if (strcmp(op, "<") == 0) {
        r = (a->cell[0]->num < a->cell[1]->num);
    }
    if (strcmp(op, ">=") == 0) {
        r = (a->cell[0]->num >= a->cell[1]->num);
    }
    if (strcmp(op, "<=") == 0) {
        r = (a->cell[0]->num <= a->cell[1]->num);
    }
    nval_del(a);
    return nval_num(r);
}

int nval_eq(nval* x, nval* y) {

  if (x->type != y->type) { return 0; }

  switch (x->type) {
    case NVAL_NUM: return (x->num == y->num);

    case NVAL_ERR: return (strcmp(x->err, y->err) == 0);
    case NVAL_SYM: return (strcmp(x->sym, y->sym) == 0);

    case NVAL_FUN:
      if (x->builtin || y->builtin) {
        return x->builtin == y->builtin;
      } else {
        return nval_eq(x->formals, y->formals) 
          && nval_eq(x->body, y->body);
      }

    case NVAL_QEXPR:
    case NVAL_SEXPR:
      if (x->count != y->count) { return 0; }
      for (int i = 0; i < x->count; i++) {
        if (!nval_eq(x->cell[i], y->cell[i])) { return 0; }
      }
      return 1;
    break;
  }
  return 0;
}

nval* builtin_cmp(nenv* e, nval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    int r;
    if (strcmp(op, "==") == 0) {
        r = nval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op, "!=") == 0) {
        r = !nval_eq(a->cell[0], a->cell[1]);
    }
    nval_del(a);
    return nval_num(r);
}

nval* builtin_eq(nenv* e, nval* a) {
    return builtin_cmp(e, a, "==");
}

nval* builtin_ne(nenv* e, nval* a) {
    return builtin_cmp(e, a, "!=");
}

nval* builtin_if(nenv* e, nval* a) {
    LASSERT_NUM("if", a, 3);
    LASSERT_TYPE("if", a, 0, NVAL_NUM);
    LASSERT_TYPE("if", a, 1, NVAL_QEXPR);
    LASSERT_TYPE("if", a, 2, NVAL_QEXPR);

    nval* x;
    a->cell[1]->type = NVAL_SEXPR;
    a->cell[2]->type = NVAL_SEXPR;

    if (a->cell[0]->num) {
        x = nval_eval(e, nval_pop(a, 1));
    } else {
        x = nval_eval(e, nval_pop(a, 2));
    }

    nval_del(a);
    return x;
}