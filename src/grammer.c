#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "util.h"
void blocks();
void exp();

void gerror(char *mesg) {
    puts(mesg);
    exit(-1);
}

void expect(char *expt, char *got) {
    printf("error: expect %s but got \"%s\"\n", expt, got);
    gerror("parsing progress has been terminated");
}

/*
 * declarations -> declaration declarations | declaration
 * or 
 * declarations -> declaration restdecl
 * restdecl -> declarations | ep
 */

void parse_program() {
    function();
}


void declarations() {
    do {
        declaration();
    } while(look_token()->type != token_end);
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
    if(token->type != variable_type)
        expect("variable type", token->value);
    move_token();

    declaration_varlist();
    if(look_token()->type != semi)
        expect(";", look_token()->value);
    move_token();
}

void declaration_varlist() {
    token *token = look_token();
    if(token->type != identifier)
        expect("identifier", token->value);
    move_token();
    rest_varlist();
}

void rest_varlist() {
    token *token = look_token();
    if(token->type != comma)
        return;
    move_token();
    token = look_token();
    if(token->type != identifier)
        expect("identifier", token->value);
    move_token();
    
    rest_varlist();
}

/*
 * function -> rettype function_name(params) { stmts }
 * rettype -> variable_type
 * function_name -> identifier
 * params -> params_list | ep
 * params_list -> param rest_params
 * rest_params -> ,params_list | ep
 * stmts -> ep | stmt stmts
 * stmt -> exp; | declaration | assgin_stmt;
 * exp -> term _exp
 * _exp -> + term _exp | - term _exp | ep
 * term -> factor _term
 * _term -> * factor _term | / factor _term | ep
 * factor -> (exp) | identifier | identifier(rparams)
 * rparams -> rparams_list | ep
 * rparams_list -> exp rest_rparams
 * rest_rparams -> ,rparams_list | ep
 * 
 * assgin_stmt -> id = exp | id += exp | id -= exp | id *= exp | id /= exp | id %= exp
 */

void e(enum token_type t, char *s) {
    if(look_token()->type != t)
        expect(s, look_token()->value);

    move_token();
}

void function() {
    e(variable_type, "variable type");
    e(identifier, "identifier");
    e(lp, "(");
    params();
    e(rp, ")");
    e(lb, "{");
    stmts();
    e(rb, "}");
}

void params() {
    if(look_token()->type != variable_type) 
        return;
    
    params_list();
}

void params_list() {
    param();
    rest_params();
}

void param() {
    e(variable_type, "variable type");
    e(identifier, "parameter");
}

void rest_params() {
    if(look_token()->type != comma) 
        return;

    move_token();
    params_list();
}

void stmts() {
    while(look_token()->type != rb)
        stmt();
}

void stmt() {
    enum token_type assign_types[] = {assign, add_assign, sub_assign, mul_assign, divi_assign};
    for(int i = 0; i < array_length(assign_types); i++) {
        if(look_n_token(2)->type == assign_types[i]) {
            assign_stmt();
            e(semi, ";");
            return;
        }
    }

    if(look_token()->type == variable_type) {
        declaration();
    } else {
        exp();
        e(semi, ";");
    }
}

void assign_stmt() {
    e(identifier, "identifier");
    move_token();
    exp();
}

void exp() {
    term();
    _exp();
}

void term() {
    factor();
    _term();
}
void _exp() {
    if(look_token()->type == add || look_token()->type == sub) {
        move_token();
        term();
        _exp();
    } else if(look_token()->type != semi && look_token()->type != rp && look_token()->type != comma) {
        expect("+ or -", look_token()->value);
    }
}

void factor() {
    if(look_token()->type == lp) {
        move_token();
        exp();
        e(rp, ")");
    } else if(look_token()->type == identifier) {
        move_token();
        if(look_token()->type == lp) {
            move_token();
            rparams();
            e(rp, ")");
        }
    } else if(look_token()->type == number) {
        move_token();
    }
}
void _term() {
    enum token_type type = look_token()->type;
    if(type == mul || type == divi) {
        move_token();
        factor();
        _term();
    } else if(type != semi && type != add && type != sub && type != comma && type != rp) {
        expect("operator", look_token()->value);
    }
}

void rparams() {
    enum token_type type = look_token()->type;
    if(type == lp || type == identifier || type == number) {
        rparams_list();
    } 
}

void rparams_list() {
    exp();
    rest_rparams();
}

void rest_rparams() {
    if(look_token()->type == comma) {
        move_token();
        rparams_list();
    }

}

int main(int argc, char **argv) {
    init_state_table();
    read_source(argv[1]);
    dfa();
    parse_program();
    puts("success");
}


