#include "ncore.h"
#ifndef nbuiltins
#define nbuiltins

/* Builtin functions and operations */
void nenv_add_builtin(nenv* e, char* name, nbuiltin func);
void nenv_add_builtins(nenv* e);
/* Arithmatic operations */
nval* builtin_op(nenv* e, nval* a, char* op);
nval* builtin_add(nenv* e, nval* a);
nval* builtin_sub(nenv* e, nval* a);
nval* builtin_mul(nenv* e, nval* a);
nval* builtin_div(nenv* e, nval* a);

/* Q-Expression functions */
nval* builtin_head(nenv* e, nval* a);
nval* builtin_tail(nenv* e, nval* a);
nval* builtin_list(nenv* e, nval* a);
nval* builtin_eval(nenv* e, nval* a);
nval* builtin_join(nenv* e, nval* a);

nval* builtin_def(nenv* e, nval* a);
nval* builtin_put(nenv* e, nval* a);
nval* builtin_var(nenv* e, nval* a, char* func);
nval* builtin_undef(nenv* e, nval* a);
nval* builtin_lambda(nenv* e, nval* a);

nval* builtin_gt(nenv* e, nval* a);
nval* builtin_lt(nenv* e, nval* a);
nval* builtin_ge(nenv* e, nval* a);
nval* builtin_le(nenv* e, nval* a);
nval* builtin_ord(nenv* e, nval* a, char* op);
int nval_eq(nval* x, nval* y);
nval* builtin_cmp(nenv* e, nval* a, char* op);
nval* builtin_eq(nenv* e, nval* a);
nval* builtin_ne(nenv* e, nval* a);
nval* builtin_if(nenv* e, nval* a);

#endif