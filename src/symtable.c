#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"

symtable *env;

void create_symbol_table() {
    symtable *s = (symtable *)malloc(sizeof(symtable));
    memset(s, 0, sizeof(symtable));
    s->prev = env;
    env = s;
}

void destory_symbol_table() {
    symtable *s = env->prev;
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
    sb->var_name[0] = '\0';
    sb->type = type;
    sb->token = t;
    linear_probe(sb, hashcode(t)); 
}

enum id_type hash_search(token *t) {
    symbol *s = symbol_search(t);
    if(s == NULL)
        return id_err;
    else
        return s->type;
}

symbol *symbol_search(token *t) {
    int hash = hashcode(t);
    for(symtable *current = env; current != NULL; current = current->prev) { 
        while(current->table[hash] != NULL) {
            if(strcmp(current->table[hash]->token->value, t->value) == 0) {
                return current->table[hash];
            } else {
                hash++;
            }
        }
    }
    return NULL;

}

symbol *symbol_shallow_search(token *t) {
    int hash = hashcode(t);
    symtable *current = env;
    while(current->table[hash] != NULL) {
            if(strcmp(current->table[hash]->token->value, t->value) == 0) {
                return current->table[hash];
            } else {
                hash++;
            }
        }
    return NULL;
}

enum id_type hash_shallow_search(token *t) {
    symbol *s = symbol_shallow_search(t);
    if(s == NULL)
        return id_err;
    else
        return s->type;
}



