#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

#define SOURCE_MAX_SIZE 1024 * 128
char source[SOURCE_MAX_SIZE];
int source_length;
int source_cursor = 0;

token *token_list[SOURCE_MAX_SIZE];
int token_length;
int token_cursor;

void print_tab();

char *keyword_set[] = {
    "char", "short", "int", "long", "float", "double", "void",
    "if", "else", "while", "for", "do", "continue", "break", "switch", "case",
    "return"
};

int finished[36];
int state_table[35][128];

void init_state_table() {
    state_table[0]['('] = -1;
    state_table[0][')'] = -2;
    state_table[0]['['] = -3;
    state_table[0][']'] = -4;
    state_table[0]['{'] = -5;
    state_table[0]['}'] = -6;
    state_table[0][','] = -7;
    state_table[0][';'] = -8;
    for (int i = 1; i <= 8; i++)
        finished[i] = 1;

    for (int i = 'a'; i <= 'z'; i++)
        state_table[0][i] = 1;
    for (int i = 'A'; i <= 'Z'; i++)
        state_table[0][i] = 1;
    state_table[0]['_'] = 1;

    for (int i = '0'; i <= '9'; i++)
        state_table[0][i] = 2;
    state_table[0]['&'] = 15;
    state_table[0]['|'] = 27;
    state_table[0]['\''] = 29;
    state_table[0]['\"'] = 32;
    state_table[0]['!'] = 25;
    state_table[0]['<'] = 23;
    state_table[0]['='] = 19;
    state_table[0]['>'] = 21;
    state_table[0]['%'] = 17;
    state_table[0]['+'] = 5;
    state_table[0]['-'] = 8;
    state_table[0]['*'] = 11;
    state_table[0]['/'] = 13;

    for (int i = 0; i < 128; i++)
        state_table[1][i] = -9;
    for (int i = 'a'; i <= 'z'; i++)
        state_table[1][i] = 1;
    for (int i = 'A'; i <= 'Z'; i++)
        state_table[1][i] = 1;
    for (int i = '0'; i <= '9'; i++)
        state_table[1][i] = 1;
    state_table[1]['_'] = 1;

    for (int i = 0; i < 128; i++)
        state_table[2][i] = -10;
    for (int i = '0'; i < '9'; i++)
        state_table[2][i] = 2;
    state_table[2]['.'] = 3;

    for (int i = '0'; i < '9'; i++)
        state_table[3][i] = 4;

    for (int i = 0; i < 128; i++)
        state_table[4][i] = -11;
    for (int i = '0'; i < '9'; i++)
        state_table[4][i] = 4;

    for (int i = 0; i < 128; i++)
        state_table[5][i] = -12;
    state_table[5]['+'] = -13;
    finished[13] = 1;
    state_table[5]['='] = -14;
    finished[14] = 1;

    for (int i = 0; i < 128; i++)
        state_table[8][i] = -15;

    state_table[8]['-'] = -16;
    finished[16] = 1;
    state_table[8]['='] = -17;
    finished[17] = 1;

    for (int i = 0; i < 128; i++)
        state_table[11][i] = -18;
    state_table[11]['='] = -19;
    finished[19] = 1;

    for (int i = 0; i < 128; i++)
        state_table[13][i] = -20;
    state_table[13]['='] = -21;
    finished[21] = 1;

    state_table[15]['&'] = -22;
    finished[22] = 1;
    for (int i = 0; i < 128; i++)
        state_table[17][i] = -23;
    state_table[17]['='] = -24;
    finished[24] = 1;

    for (int i = 0; i < 128; i++)
        state_table[19][i] = -25;
    state_table[19]['='] = -26;
    finished[26] = 1;

    for (int i = 0; i < 128; i++)
        state_table[21][i] = -27;
    state_table[21]['='] = -28;
    finished[28] = 1;

    for (int i = 0; i < 128; i++)
        state_table[23][i] = -29;
    state_table[23]['='] = -30;

    for (int i = 0; i < 128; i++)
        state_table[25][i] = -31;
    state_table[25]['='] = -32;
    finished[32] = 1;

    state_table[27]['|'] = -33;
    finished[33] = 1;

    for (int i = 0; i < 128; i++)
        state_table[29][i] = 30;
    state_table[30]['\''] = -34;
    finished[34] = 1;
    for (int i = 0; i < 128; i++)
        state_table[32][i] = state_table[33][i] = 33;
    state_table[32]['\"'] = state_table[33]['\"'] = -35;
    finished[35] = 1;
}

enum token_type is_keyword(char *ident) {
    for (int i = 0; i < sizeof(keyword_set) / sizeof(char *); i++) {
          if (strcmp(ident, keyword_set[i]) == 0) {
                if (i <= 6)
                    return variable_type;
                else return keyword;
            }
      }
    return identifier;
}


int read_source(char *source_name) {
    FILE *source_file = fopen(source_name, "r");

    if (source_file == NULL)
        return 0;
    source_length = fread(source, sizeof(char), SOURCE_MAX_SIZE, source_file);
    fclose(source_file);
    return 1;
}

int look_char() {
    if (source_cursor >= source_length)
        return EOF;
    else
        return source[source_cursor];
}

void move_char() {
    source_cursor++;
}

void move_token() {
    token_cursor++;
}

token *look_token() {
    if (token_cursor >= token_length)
        return token_list[token_length];
    else
        return token_list[token_cursor];
}

token *look_n_token(int n) {
    if (token_cursor + n - 1 >= token_length) {
          return token_list[token_length];
      } else
        return token_list[token_cursor + n - 1];
}

void insert_token(token *t) {
    token_list[token_length++] = t;
}

void create_token_value(token *new_token, int start, int end) {
    int i = 0;

    for (; start <= end; i++, start++) {
          new_token->value[i] = source[start];
      }
    new_token->value[i] = '\0';
}

void generate_token(int type, int line, int start, int col) {
    token *t = (token*)malloc(sizeof(token));

    t->value[0] = 0;
    switch (type) {
      case 0:
          t->type = token_end; break;

      case -1:
          t->type = lp; break;

      case -2:
          t->type = rp; break;

      case -3:
          t->type = lsb; break;

      case -4:
          t->type = rsb; break;

      case -5:
          t->type = lb; break;

      case -6:
          t->type = rb; break;

      case -7:
          t->type = comma; break;

      case -8:
          t->type = semi; break;

      case -9:
          t->type = identifier;
          break;

      case -10:
          t->type = number;
          break;

      case -11:
          t->type = real;
          break;

      case -12:
          t->type = add; break;

      case -13:
          t->type = self_add; break;

      case -14:
          t->type = add_assign; break;

      case -15:
          t->type = sub; break;

      case -16:
          t->type = self_sub; break;

      case -17:
          t->type = sub_assign; break;

      case -18:
          t->type = mul; break;

      case -19:
          t->type = mul_assign; break;

      case -20:
          t->type = divi; break;

      case -21:
          t->type = divi_assign; break;

      case -22:
          t->type = and; break;

      case -23:
          t->type = mod; break;

      case -24:
          t->type = mod_assign; break;

      case -25:
          t->type = assign; break;

      case -26:
          t->type = equal; break;

      case -27:
          t->type = gt; break;

      case -28:
          t->type = get; break;

      case -29:
          t->type = lt; break;

      case -30:
          t->type = let; break;

      case -31:
          t->type = not; break;

      case -32:
          t->type = nequal; break;

      case -33:
          t->type = or; break;

      case -34:
          t->type = single_char;
          create_token_value(t, start + 1, start + 1);
          break;

      case -35:
          t->type = string;
          create_token_value(t, start + 1, source_cursor - 2);
          break;
      }
    if (type > -34)
        create_token_value(t, start, source_cursor - 1);
    if (t->type == identifier) {
          t->type = is_keyword(t->value);
      }
    t->loc.line = line;
    t->loc.col = col;

    insert_token(t);
}



void error(int line, int col) {
    printf("error: lexical error, line:%d, col:%d\n", line, col);
    exit(0);
}

void dfa() {
    int state = 0;
    int start = source_cursor;
    int line = 1;
    int col = 1;

    while (1) {
          int c = look_char();
          if (c == EOF) {
                if (state < 0)
                    generate_token(state, line, start, col);

                generate_token(0, line, start, col);
                break;
            } else if ((c == '\n' || c == ' ' || c == '\t') && state == 0) {
                if (c == '\n') {
                    line++;
                    col = 1;
                  }
                move_char();
                col++;

                start = source_cursor;
                continue;
            }


          state = state_table[state][c];
          if (state < 0) {
                if (finished[-state]) {
                    move_char();
                    col++;
                  }
                generate_token(state, line, start, col);
                state = 0;
                start = source_cursor;
            } else if (state == 0) {
                error(line, col);
            } else {
                move_char();
                col++;
            }
      }
}
