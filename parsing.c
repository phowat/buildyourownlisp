#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char * unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Creates new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Creates new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

void lval_print(lval v) {
  switch (v.type) {
  case LVAL_NUM: 
    printf("%li", v.num);
    break;

  case LVAL_ERR:
    if(v.err == LERR_DIV_ZERO) {
      printf("Error: Division By Zero!");
    } else if(v.err == LERR_BAD_OP) {
      printf("Error: Invalid Operator!");
    } else if(v.err == LERR_BAD_NUM) {
      printf("Error: Invalid Number!");
    }
    break;
  }
}

void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}

long exponential(long i, long n, long buf) {
  if (n == 0) {
    return 1; 
  } else if (n == 1) {
    return buf; 
  } else {
    return exponential(i, n-1, buf*i);
  }
}

long minVal(long x, long y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

long maxVal(long x, long y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}

/* Use operator string to see which operation to perform. */
long eval_single_arg(long x, char* op) {
  if (strcmp(op, "-") == 0) { return - x; }
  return x;
}
/* Use operator string to see which operation to perform. */
lval eval_op(lval x, char* op, lval y) {
  
  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  /* Otherwise do maths on the number values */
  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) { 
    /* If second operand is zero return error */
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }
  /* TODO: Adapt the custom operators from previous chapter exercises */
  /*
  if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
  if (strcmp(op, "^") == 0) { return exponential(x, y, x); }
  if (strcmp(op, "min") == 0) { return minVal(x, y); }
  if (strcmp(op, "max") == 0) { return maxVal(x, y); }
  */
  return lval_err(LERR_BAD_OP);
}

long xxx(mpc_ast_t* t) {

  int i = 0;
  printf("T, TAG = %s, CONTENTS = %s.\n", t->tag, t->contents);
  while(t->children[i]) {
    printf("I = %d, TAG = %s, CONTENTS = %s.\n", i, t->children[i]->tag, t->children[i]->contents);
    i++;
  }
  return 5;
}

int count(mpc_ast_t* t) {

  /* If tagged as number return it directly */
  if (strstr(t->tag, "number")) {
    return 1;
  }
  
  int ret = 1; // At least children[1] will be a leaf at this point.
  int i = 2;
  while (strstr(t->children[i]->tag, "expr")) {
    ret += count(t->children[i]);
    i++;
  }

  return ret;
}

lval eval(mpc_ast_t* t) {

  /* If tagged as number return it directly */
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }
  
  /* The oprator is always second child. */
  char *op = t->children[1]->contents;

  /* We store the third in `x` */
  lval x = eval(t->children[2]);

  /* Iterate the remaining children and combining. */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  /* TODO: Update eval_single_arg */
  /*
  if (i == 3) {
    x = eval_single_arg(x, op);
  }
  */

  return x;
}

int main(int argc, char** argv) {
  
  /* Create Some Parsers */
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define them with the following language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                    \
      number   : /-?[0-9]+/ ;                            \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ; \
      lispy    : /^/ <operator> <expr>+ /$/ ;            \
    ",
    Number, Operator, Expr, Lispy);
  
  puts("Lisphry Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit.\n");

  while (1) {
    char * input = readline("lisphry> ");
    add_history(input);
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* On Success Print the AST */
      /*
      mpc_ast_print(r.output);
      int result = count(r.output);
      printf("%d\n", result);
      */
      lval result = eval(r.output);
      //printf("%li\n", result);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}

/*
 *
 *
 * The purpose of typedef is to form complex types from more-basic machine types[1] and assign
 * simpler names to such combinations
 *
 */

/* Chapter 8
 *  >  How do you give an enum a name?
 *  enum toilet_actions {
 *    pee,
 *    poop,
 *    shower,
 *    brush_teeth
 *  };
 */

