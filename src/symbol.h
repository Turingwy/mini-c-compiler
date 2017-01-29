#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "token.h"

#define table_size 1024

enum id_type {
    id_nul, id_char, id_short, id_int, id_long, id_float, id_err
};

typedef struct symbol {
    token *token;
    char var_name[16];
    enum id_type type;
} symbol;

typedef struct symtable {
    symbol *table[table_size];
    int length;
    struct symtable *prev;
} symtable;

extern symtable *env;

void create_symbol_table();
void destory_symbol_table();
void hash_input(token *t, enum id_type type);
enum id_type hash_search(token *t);
symbol *symbol_search(token *t);
enum id_type hash_shallow_search(token *t);
symbol *symbol_shallow_search(token *t);
#endif
