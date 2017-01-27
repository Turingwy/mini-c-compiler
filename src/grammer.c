#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "symbol.h"
#include "util.h"

void block();
void print_tab();
void if_stmt();
void rest_if();
void _block();
void read_source();
void init_state_table();
enum id_type _exp();
enum id_type _term();
void assign_stmt();
void declaration();
void declaration_varlist();
void declarations();
void e(enum token_type t, char *s);
void type_error();
enum id_type expression();
void expect(char *expt, char *got);
enum id_type factor();
void function();
void gerror(char *mesg);
void param();
void params();
void params_list();
void parse_program();
void rest_params();
void rest_rparams();
void rest_varlist();
void rparams();
void rparams_list();
void stmt();
void stmts();
enum id_type term();
void dfa();
void while_stmt();
enum id_type logical_exp();
enum id_type and_exp();
enum id_type _logical_exp();
enum id_type comparision_exp();
enum id_type _and_exp();
void rest_factor();

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

void parse_program() {
    create_symbol_table();
    function();
    destory_symbol_table();
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
    if(hash_search(token) != id_err) 
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
    if(hash_search(token) != id_err) 
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

void function() {
    create_symbol_table();
    e(variable_type, "variable type");
    e(identifier, "identifier");
    e(lp, "(");
    params();
    e(rp, ")");
    _block();
    destory_symbol_table();
}

void block() {
    create_symbol_table();
    _block();
    destory_symbol_table();
}

void _block() {
    e(lb, "{");
    stmts();
    e(rb, "}");
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

void stmts() {
    while (look_token()->type != rb && look_token()->type != token_end) {
          stmt();
      }
}

void stmt() {
    enum token_type assign_types[] = { assign, add_assign, sub_assign, mul_assign, divi_assign };

    for (int i = 0; i < array_length(assign_types); i++) {
          if (look_n_token(2)->type == assign_types[i]) {
                assign_stmt();
                e(semi, ";");
                return;
            }
      }

    if (look_token()->type == variable_type) {
          declaration();
      } else if (look_token()->type == lb) {
          block();
      } else if (strcmp(look_token()->value, "if") == 0) {
          if_stmt();
      } else if (strcmp(look_token()->value, "while") == 0) {
          while_stmt();
      } else {
          logical_exp();
          e(semi, ";");
      }
}

void type_error() {
    gerror("type error");
}

void assign_stmt() {
    token *t = look_token();
    e(identifier, "identifier");
    enum id_type type = hash_search(t);
    if(type == id_err)
       type_error(); 
    move_token();
    if(logical_exp(type) > type)
        type_error();
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


enum id_type expression() {
    enum id_type left_type = term();
    enum id_type right_type = _exp();
    return arith_type(left_type, right_type);
}

enum id_type term() {
    enum id_type left_type = factor();
    enum id_type right_type = _term();
    enum id_type type = arith_type(left_type, right_type);
    return type;
}

enum id_type _exp() {
    enum token_type type = look_token()->type;
    if (type == add || type == sub) {
          move_token();
          enum id_type lt = term();
          return arith_type(lt, _exp());
      } else if (type != semi && type != rp && type != comma
                 && type != and && type != or && type != equal && type != nequal && type != lt && type != gt && type != let && type != get
                 ) {
          expect("+ or -", look_token()->value);
      }
    return id_nul;
}

enum id_type factor() {
    if (look_token()->type == lp) {
          move_token();
          enum id_type type = logical_exp();
          e(rp, ")");
          return type;
      } else if (look_token()->type == identifier) {        // TODO: function return type
          enum id_type type = hash_search(look_token());
          if(type == id_err)
              type_error();
          move_token();
          rest_factor();
          return type;
      }else if (look_token()->type == number) {
          move_token();
          return id_int;
      } /*else if (look_token()->type == string) {
          move_token();
      }*/
        else if (look_token()->type == self_add || look_token()->type == self_sub) {
          move_token();
          enum id_type type = hash_search(look_token());
          if(type == id_err)
              type_error();
          e(identifier, "identifier");
          return type;
      } else {
          expect("factor", look_token()->value);
      }
}

void rest_factor() {
    enum token_type type = look_token()->type;

    switch (type) {
      case self_add:
      case self_sub:
          move_token(); break;

      case lp:
          move_token();
          rparams();
          e(rp, ")");
          break;

      default:
          break;
      }
}

enum id_type _term() {
    enum token_type type = look_token()->type;

    if (type == mul || type == divi) {
          move_token();
          enum id_type left_type = factor();
          enum id_type right_type = _term();
          return arith_type(left_type, right_type);
      } else if (type != semi && type != add && type != sub && type != comma && type != rp
                 && type != and && type != or && type != equal && type != nequal && type != lt && type != gt && type != let && type != get) {
          expect("operator", look_token()->value);
      }
    return id_nul;
}

void rparams() {
    enum token_type type = look_token()->type;
    
    if (type != rp) {
          rparams_list();
      }
}

void rparams_list() {
    expression();
    rest_rparams();
}

void rest_rparams() {
    if (look_token()->type == comma) {
          move_token();
          rparams_list();
      }
}

void if_stmt() {
    token *t = look_token();

    if (strcmp(t->value, "if") == 0) {
          move_token();
          e(lp, "(");
          if(logical_exp() == id_nul)
              type_error();
          e(rp, ")");
          if(look_token()->type == lb) {
            block();
          } else {
              create_symbol_table();
              stmt();
              destory_symbol_table();
          }
          rest_if();
      }
}

void rest_if() {
    token *t = look_token();

    if (strcmp(t->value, "else") == 0) {
          move_token();
          stmt();
      }
}

void while_stmt() {
    token *t = look_token();

    if (strcmp(t->value, "while") == 0) {
          move_token();
          e(lp, "(");
          logical_exp();
          e(rp, ")");
          stmt();
      }
}

enum id_type comparision_exp() {
    enum id_type left_type = expression();
    token *token = look_token();
    enum token_type cprs_symbols[] = { equal, lt, gt, let, get, nequal };

    for (size_t i = 0; i < 6; i++) {
          if (cprs_symbols[i] == token->type) {
                move_token();
                return arith_type(left_type, expression());
            }
      }
    return left_type;
}

enum id_type logical_exp() {
    enum id_type lt = and_exp();
    return arith_type(lt, _logical_exp());
}

enum id_type _logical_exp() {
    if (look_token()->type != or)
        return id_nul;
    e(or, "||");
    enum id_type lt = and_exp();
    return arith_type(lt, _logical_exp());
}

enum id_type and_exp() {
    enum id_type lt = comparision_exp();
    return arith_type(lt, _and_exp());
}

enum id_type _and_exp() {
    if (look_token()->type != and) {
          return id_nul;
      }
    e(and, "&&");
    enum id_type lt = comparision_exp();
    return arith_type(lt,_and_exp());
}

int main(int argc, char **argv) {
    if (argc != 2)
        printf("minic: Usage: minic [file].c\n");
    init_state_table();
    read_source(argv[1]);
    dfa();
    parse_program();
    puts("success");
}
