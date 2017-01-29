#ifndef _AST_H_
#define _AST_H_


#include "token.h"
#include "symbol.h"

enum ast_type {
    ast_type_program,
    ast_type_function,
    ast_type_seq, 
    ast_type_if,
    ast_type_while,
    ast_type_assign,
    ast_type_or,
    ast_type_and,
    ast_type_equal,
    ast_type_nequal,
    ast_type_lt,
    ast_type_gt,
    ast_type_let,
    ast_type_get,
    ast_type_call,
    ast_type_add,
    ast_type_sub,
    ast_type_mul,
    ast_type_divi,
    ast_type_self_add,
    ast_type_self_sub,
    ast_type_num,
    ast_type_id
};

typedef struct ast_node {
    enum ast_type type;

} ast_node;

typedef struct ast_node_var {
   enum ast_type type;
   char var_name[16];
} ast_node_var;

typedef struct ast_program {
    enum ast_type type;
    ast_node *func;
} ast_program;

typedef struct ast_function {
    enum ast_type type;
    enum id_type rettype;
    ast_node *block_seq;
} ast_function;

typedef struct ast_seq {
    enum ast_type type;
    ast_node *stmt;
    ast_node *rest;
} ast_seq;

typedef struct ast_if {
    enum ast_type type;
    ast_node *exp;
    ast_node *seq;
    ast_node *else_seq;
} ast_if;

typedef struct ast_while {
    enum ast_type type;
    ast_node *exp;
    ast_node *loop_seq;
} ast_while;

typedef struct ast_assign {
    enum ast_type type;
    ast_node *id;
    ast_node *exp;
} ast_assign;

typedef struct ast_exp {
    enum ast_type type;
    char var_name[16];
    ast_node *left;
    ast_node *right;
} ast_exp;

typedef struct ast_num {
    enum ast_type type;
    char var_name[16];
    char value[16];
} ast_num;

typedef struct ast_id {
    enum ast_type type;
//    char var_name[16];
    symbol *id_symbol;
} ast_id;

typedef struct ast_self_op {
    enum ast_type type;
    char var_name[16];
    int first;
    ast_id *id;
} ast_self_op;

void translate_to_3addr(ast_node *);

#endif

