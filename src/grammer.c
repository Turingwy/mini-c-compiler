#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "symbol.h"
#include "util.h"
#include "ast.h"

ast_node *block();
void print_tab();
ast_node *if_stmt();
ast_node *rest_if();
ast_node *_block();
ast_node *read_source();
ast_node *init_state_table();
enum id_type _exp(ast_node *left, ast_node **node);
enum id_type _term(ast_node *left, ast_node **node);
ast_node *assign_stmt();
void declaration();
void declaration_varlist();
void declarations();
void e(enum token_type t, char *s);
void type_error();
enum id_type expression(ast_node **node);
void expect(char *expt, char *got);
enum id_type factor(ast_node **node);
ast_node *function();
void gerror(char *mesg);
void param();
void params();
void params_list();
ast_program *parse_program();
void rest_params();
void rest_rparams();
void rest_varlist();
void rparams();
void rparams_list();
ast_node *stmt();
ast_node *stmts();
enum id_type term(ast_node **node);
void dfa();
ast_node *while_stmt();
enum id_type logical_exp(ast_node **node);
enum id_type and_exp(ast_node **node);
enum id_type _logical_exp(ast_node *left, ast_node **node);
enum id_type comparision_exp(ast_node **node);
enum id_type _and_exp(ast_node *left, ast_node **node);
void rest_factor(ast_id *id, ast_node **node);

void gerror(char *mesg) {
    puts(mesg);
    exit(-1);
}

void expect(char *expt, char *got) {
    printf("error: expect %s but got \"%s\", line %d, col %d\n", expt, got, look_token()->loc.line, look_token()->loc.col);
    gerror("parsing progress has been terminated");
}

enum id_type getType(token *t) {
    switch(t->value[0]) {
        case 'i':
            return id_int;
        case 'c':
            return id_char;
        case 's':
            return id_short;
        case 'l':
            return id_long;
        case 'f':
            return id_float;
        default:
            return id_err;
    }
}

/*
 * declarations -> declaration declarations | declaration
 * or
 * declarations -> declaration restdecl
 * restdecl -> declarations | ep
 */

ast_program *parse_program() {
    ast_program *syn = (ast_program *)malloc(sizeof(ast_program));
    syn->type = ast_type_program;
    create_symbol_table();
    syn->func = function();
    destory_symbol_table();
    return syn;
}


void declarations() {
    do {
          declaration();
      } while (look_token()->type != token_end);
}

/*
 * declaration -> variable_type declaration_varlist ;
 * declaration_varlist -> declaration_varlist, identifier | identifier
 * eliminate left recursive:
 * declaration_varlist -> identifier rest_varlist
 * rest_varlist -> ,identifier rest_varlist | ep
 */
void declaration() {
    token *token = look_token();

    if (token->type != variable_type)
        expect("variable type", token->value);
    move_token();
    enum id_type type = getType(token);
     
    declaration_varlist(type);
    if (look_token()->type != semi)
        expect(";", look_token()->value);
    move_token();
}

void declaration_varlist(enum id_type type) {
    token *token = look_token();

    if (token->type != identifier)
        expect("identifier", token->value);
    if(hash_shallow_search(token) != id_err) 
        type_error();
    hash_input(token, type);
    move_token();
    rest_varlist(type);
}

void rest_varlist(enum id_type type) {
    token *token = look_token();

    if (token->type != comma)
        return;

    move_token();
    token = look_token();
    if (token->type != identifier)
        expect("identifier", token->value);
    if(hash_shallow_search(token) != id_err) 
        type_error();
    hash_input(token, type);
    move_token();

    rest_varlist(type);
}

/*
 * function -> rettype function_name(params) blocks
 * rettype -> variable_type
 * function_name -> identifier
 * params -> params_list | ep
 * params_list -> param rest_params
 * rest_params -> ,params_list | ep
 * stmts -> ep | stmt stmts
 * stmt -> exp; | declaration | assgin_stmt; | block | if_stmt | while_stmt
 * blocks -> { stmts }
 * logical_exp -> and_exp _logical_exp
 * _logical_exp -> || and_exp _logical_exp | ep
 * and_exp -> comparison_exp _and_exp
 * _and_exp -> && comparison_exp _and_exp | ep
 * comparison_exp -> exp == exp | exp != exp | exp >= exp | exp <= exp | exp > exp | exp < exp | exp
 * exp -> term _exp
 * _exp -> + term _exp | - term _exp | ep
 * term -> factor _term
 * _term -> * factor _term | / factor _term | ep
 * factor -> (exp) | identifier rest_factor | ++identifier | --identifier | string | single_char
 * rest_factor -> ep | ++ | -- | (rparams)
 * rparams -> rparams_list | ep
 * rparams_list -> exp rest_rparams
 * rest_rparams -> ,rparams_list | ep
 * assgin_stmt -> id = exp | id += exp | id -= exp | id *= exp | id /= exp | id %= exp
 * if_stmt -> if(exp) stmt rest_if
 * rest_if -> ep | else stmt
 * while_stmt -> while(exp) stmt
 *
 */

void e(enum token_type t, char *s) {
    if (look_token()->type != t)
        expect(s, look_token()->value);
    move_token();
}

ast_node *function() {
    ast_function *syn = (ast_function *)malloc(sizeof(ast_function));
    syn->type = ast_type_function;
    create_symbol_table();
    syn->rettype = getType(look_token());
    e(variable_type, "variable type");
    e(identifier, "identifier");
    e(lp, "(");
    params();
    e(rp, ")");
    syn->block_seq = _block();
    destory_symbol_table();
    return (ast_node *)syn;
}

ast_node *block() {
    ast_node *syn;
    create_symbol_table();
    syn = _block();
    destory_symbol_table();
    return syn;
}

ast_node *_block() {
    e(lb, "{");
    ast_node *syn = stmts();
    e(rb, "}");
    return syn;
}

void params() {
    if (look_token()->type != variable_type)
        return;

    params_list();
}

void params_list() {
    param();
    rest_params();
}

void param() {
    token *t = look_token(); 
    e(variable_type, "variable type");
    enum id_type type = getType(t);
    t = look_token();
    e(identifier, "parameter");
    hash_input(t, type);
}

void rest_params() {
    if (look_token()->type != comma)
        return;

    move_token();
    params_list();
}

ast_node *stmts() {
    if(look_token()->type == rb || look_token()->type == token_end)
        return NULL;
    ast_seq *syn = (ast_seq *)malloc(sizeof(ast_seq));
    syn->type = ast_type_seq;
    syn->stmt = stmt();
    syn->rest = stmts();
    return (ast_node *)syn;

}

ast_node *stmt() {
    enum token_type assign_types[] = { assign, add_assign, sub_assign, mul_assign, divi_assign };

    for (int i = 0; i < array_length(assign_types); i++) {
          if (look_n_token(2)->type == assign_types[i]) {
                ast_node *syn = assign_stmt();
                e(semi, ";");
                return syn;
            }
      }

    if (look_token()->type == variable_type) {
          declaration();
      } else if (look_token()->type == lb) {
          return block();
      } else if (strcmp(look_token()->value, "if") == 0) {
          return if_stmt();
      } else if (strcmp(look_token()->value, "while") == 0) {
          return while_stmt();
      } else {
          ast_node *node = NULL;
          logical_exp(&node);
          e(semi, ";");
          return node;
      }
    return NULL;
}

void type_error() {
    gerror("type error");
}

ast_node *assign_stmt() {
    ast_assign *syn = (ast_assign *)malloc(sizeof(ast_assign));
    syn->type = ast_type_assign;
    token *t = look_token();
    syn->id = (ast_node *)malloc(sizeof(ast_id));
    ast_id *id = (ast_id *)syn->id;
    id->type = ast_type_id;
    e(identifier, "identifier");
    id->id_symbol = symbol_search(t);
    if(id->id_symbol == NULL)
       type_error(); 
    move_token();
    if(logical_exp(((ast_node **)(&(syn->exp)))) > id->id_symbol->type)
        type_error();
    return (ast_node *)syn;
}

enum id_type arith_type(enum id_type left_type, enum id_type right_type) {
    if(left_type == id_err || right_type == id_err)
        return id_err;
    enum id_type max_type;
    if(left_type != id_float && right_type != id_float) {
        max_type = left_type > right_type ? left_type : right_type;
        if(max_type != id_long)
            return id_int;
        else
            return id_long;
    } else
        return id_float;

}


enum id_type expression(ast_node **node) {
    ast_node *left = NULL;
    enum id_type left_type = term(&left);
    enum id_type right_type = _exp(left, node);
    return arith_type(left_type, right_type);
}

enum id_type term(ast_node **node) {
    ast_node *left = NULL;
    enum id_type left_type = factor(&left);
    enum id_type right_type = _term(left, node);
    return arith_type(left_type, right_type);
}

enum id_type _exp(ast_node *left, ast_node **node) {
    enum token_type type = look_token()->type;
    if (type == add || type == sub) {
          move_token();
          ast_exp *as = (ast_exp *)malloc(sizeof(ast_exp));
          if(type == add)
              as->type = ast_type_add;
          else
              as->type = ast_type_sub;

          as->left = left;
          enum id_type lt = term(&(as->right));
          return arith_type(lt, _exp((ast_node *)as, node));
      } else if (type != semi && type != rp && type != comma
                 && type != and && type != or && type != equal && type != nequal && type != lt && type != gt && type != let && type != get
                 ) {
          expect("+ or -", look_token()->value);
      }
    *node = left;
    return id_nul;
}

enum id_type factor(ast_node **node) {
    if (look_token()->type == lp) {
          move_token();
          enum id_type type = logical_exp(node);
          e(rp, ")");
          return type;
      } else if (look_token()->type == identifier) {        // TODO: function return type
          enum id_type type = hash_search(look_token());
          if(type == id_err)
              type_error();
          ast_id *id = (ast_id *)malloc(sizeof(ast_id));
          id->type = ast_type_id;
          id->id_symbol = symbol_search(look_token());
          move_token();
          rest_factor(id, node);
          return type;
      }else if (look_token()->type == number) {
          ast_num *num = (ast_num *)malloc(sizeof(ast_num));
          num->type = ast_type_num;
          strcpy(num->value, look_token()->value);
          *node = (ast_node *)num;
          move_token();
          return id_int;
      } /*else if (look_token()->type == string) {
          move_token();
      }*/
        else if (look_token()->type == self_add || look_token()->type == self_sub) {
          move_token();
          ast_self_op *self = (ast_self_op *)malloc(sizeof(ast_self_op));
          self->first = 1;
          if(look_token()->type == self_add)
              self->type = ast_type_self_add;
          else 
              self->type = ast_type_self_sub;
          enum id_type type = hash_search(look_token());
          if(type == id_err)
              type_error();
          self->id = (ast_id *)malloc(sizeof(ast_id));
          self->id->id_symbol = symbol_search(look_token());
          self->id->type = ast_type_id;
          *node = (ast_node *)self;
          e(identifier, "identifier");
          return type;
      } else {
          expect("factor", look_token()->value);
      }
    return id_nul;
}

void rest_factor(ast_id *id, ast_node **node) {
    enum token_type type = look_token()->type;
    ast_self_op *self = (ast_self_op *)malloc(sizeof(ast_self_op));
    switch (type) {
      case self_add:
          self->id = id;
          self->first = 0;
          self->type = ast_type_self_add;
          *node = (ast_node *)self;
          move_token();
          break;
      case self_sub:
          self->id = id;
          self->first = 0;
          self->type = ast_type_self_sub;
          *node = (ast_node *)self;
          move_token(); break;

      case lp:          // symbol table and tree TODO
          move_token();
          rparams();
          e(rp, ")");
          break;

      default:
          *node = (ast_node *)id;
          break;
      }
}

enum id_type _term(ast_node *left, ast_node **node) {
    enum token_type type = look_token()->type;

    if (type == mul || type == divi) {
          move_token();
          ast_exp *md = (ast_exp *)malloc(sizeof(ast_exp));
          if(type == mul)
              md->type = ast_type_mul;
          else
              md->type = ast_type_divi;
          md->left = left;
          enum id_type left_type = factor(&(md->right));
          enum id_type right_type = _term((ast_node *)md, node);
          return arith_type(left_type, right_type);
      } else if (type != semi && type != add && type != sub && type != comma && type != rp
                 && type != and && type != or && type != equal && type != nequal && type != lt && type != gt && type != let && type != get) {
          expect("operator", look_token()->value);
      }
    *node = left;
    return id_nul;
}

void rparams() {
    enum token_type type = look_token()->type;
    
    if (type != rp) {
          rparams_list();
      }
}

void rparams_list() {
    ast_node *node = NULL;
    expression(&node);
    rest_rparams();
}

void rest_rparams() {
    if (look_token()->type == comma) {
          move_token();
          rparams_list();
      }
}

ast_node *if_stmt() {
    token *t = look_token();

    if (strcmp(t->value, "if") == 0) {
          ast_if *if_node = (ast_if *)malloc(sizeof(ast_if));
          if_node->type = ast_type_if;
          move_token();
          e(lp, "(");
          if(logical_exp(&(if_node->exp)) == id_nul)
              type_error();
          e(rp, ")");
          if(look_token()->type == lb) {
            if_node->seq = block();
          } else {
              create_symbol_table();
              if_node->seq = stmt();
              destory_symbol_table();
          }
          if_node->else_seq = rest_if();

          return (ast_node *)if_node;
      }
    return NULL;
}

ast_node *rest_if() {
    token *t = look_token();

    if (strcmp(t->value, "else") == 0) {
          move_token();
          return stmt();
      }
    return NULL;
}

ast_node *while_stmt() {
    token *t = look_token();
    
    if (strcmp(t->value, "while") == 0) {
          ast_while *while_node = (ast_while *)malloc(sizeof(ast_while));
          while_node->type = ast_type_while;
          move_token();
          e(lp, "(");
          logical_exp(&(while_node->exp));
          e(rp, ")");
          while_node->loop_seq = stmt();
          return (ast_node *)while_node;
      }
    return NULL;
}

enum id_type comparision_exp(ast_node **node) {
    ast_node *left = NULL, *right = NULL;
    enum id_type left_type = expression(&left);
    token *token = look_token();
    enum token_type cprs_symbols[] = { equal, lt, gt, let, get, nequal };

    for (size_t i = 0; i < 6; i++) {
          if (cprs_symbols[i] == token->type) {
                move_token();
                enum id_type rettype = arith_type(left_type, expression(&right));
                ast_exp *cprs_exp = (ast_exp *)malloc(sizeof(ast_exp));
                cprs_exp->left = left;
                cprs_exp->right = right;
                switch (cprs_symbols[i]) {
                    case equal:
                        cprs_exp->type = ast_type_equal;
                        break;
                    case lt:
                        cprs_exp->type = ast_type_lt;
                        break;
                    case gt:
                        cprs_exp->type = ast_type_gt;
                        break;
                    case let:
                        cprs_exp->type = ast_type_let;
                        break;
                    case get:
                        cprs_exp->type = ast_type_get;
                        break;
                    case nequal:
                        cprs_exp->type = ast_type_nequal;
                        break;
                    default:
                        break;
                }
                *node = (ast_node *)cprs_exp;
                return rettype;
            }
      }
    *node = left;
    return left_type;
}

enum id_type logical_exp(ast_node **node) {
    ast_node *node1 = NULL;
    enum id_type lt = and_exp(&node1);
    enum id_type rettype = arith_type(lt, _logical_exp(node1, node));
     
    return rettype;
}

enum id_type _logical_exp(ast_node *first, ast_node **node) {
    if (look_token()->type != or) {
        *node = first;
        return id_nul;
    }
    e(or, "||");
    ast_exp *or = (ast_exp *)malloc(sizeof(ast_exp));
    or->type = ast_type_or;
    or->left = first;
    enum id_type lt = and_exp(&(or->right));

    return arith_type(lt, _logical_exp((ast_node *)or, node));

}

enum id_type and_exp(ast_node **node) {
    ast_node *left = NULL;
    enum id_type lt = comparision_exp(&left);
    return arith_type(lt, _and_exp(left, node));
}

enum id_type _and_exp(ast_node *left, ast_node **node) {
    if (look_token()->type != and) {
          *node = left;
          return id_nul;
      }
    e(and, "&&");
    ast_exp *and = (ast_exp *)malloc(sizeof(ast_exp));
    and->type = ast_type_and;
    and->left = left;
    enum id_type lt = comparision_exp(&(and->right));
    return arith_type(lt,_and_exp((ast_node *)and, node));
}

int main(int argc, char **argv) {
    if (argc != 2)
        printf("minic: Usage: minic [file].c\n");
    init_state_table();
    read_source(argv[1]);
    dfa();
    ast_program *ast_tree = parse_program();
    translate_to_3addr((ast_node *)ast_tree);
    puts("success");
}
