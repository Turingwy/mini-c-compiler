#ifndef _TOKEN_H_
#define _TOKEN_H_

#define TOKEN_VALUE_SIZE 16

enum token_type {
        identifier,
        variable_type,
        keyword,        // like int while
        number,         // 1234
        real,           // 12.2
        single_char,    // 'a'
        string,         // "hello world"
        lp, rp,         // ( and )
        lsb, rsb,       // [ and ]
        lb, rb,         // { and }
        assign,         // =
        comma, semi,         // ,  ;
        equal, nequal, lt, gt, let, get, // ==, !=, <, >, <=, >=
        add, sub, mul, divi, mod,  // +, -, *, /, %
        self_add, self_sub,    // ++, --
        add_assign, sub_assign, mul_assign, divi_assign, mod_assign,      // +=, -=, *=, /=, %=
        and, or, not  // &&, ||, !
        ,token_end
};

typedef struct token_loc {
    int col;
    int line;
} token_loc;

typedef struct token {
    enum token_type type;
    char value[TOKEN_VALUE_SIZE];
    token_loc loc;
} token;

token *look_token();
void move_token();
token *look_n_token(int n);
#endif
