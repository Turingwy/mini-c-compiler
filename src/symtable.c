#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"

symtable *env;

void create_symbol_table() {
    symtable *s = (symtable *)malloc(sizeof(symtable));
    s->prev = env;
    env = s;
}

void destory_symbol_table() {
    symtable *s = env->prev;
    for(int i = 0; i < env->length; i++) {
        free(env->table[i]);
    }
    free(env);
    env = s;
}

int hashcode(token *t) {
    int i = 0;
    int hash = 0;
    while(t->value[i] != '\0') {
        hash = hash*17+11*t->value[i++];
        hash %= table_size;
    }
    return hash;
}


void linear_probe(symbol *symbol, int hash) {
    while(env->table[hash] != NULL) {
        hash++;
        hash%=table_size;
    }
    env->table[hash] = symbol;
            
}

void hash_input(token *t, enum id_type type) {
    symbol *sb = (symbol *)malloc(sizeof(symbol));

    sb->type = type;
    sb->token = t;
    linear_probe(sb, hashcode(t)); 
}

enum id_type hash_search(token *t) {
    int hash = hashcode(t);
    while(env->table[hash] != NULL) {
        if(strcmp(env->table[hash]->token->value, t->value) == 0) {
            return env->table[hash]->type;
        } else {
            hash++;
        }
    }
    return id_err;
}

