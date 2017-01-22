#ifndef _TOKEN_H_
#define _TOKEN_H_

#define TOKEN_VALUE_SIZE 16

enum token_type {
        identifier,
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
};

typedef struct token {
    enum token_type type;
    char value[TOKEN_VALUE_SIZE];
} token;


token *look_token();
token *next_token();

#endif
