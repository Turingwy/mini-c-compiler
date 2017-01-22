#include "token.h"
#include <stdio.h>

void init_state_table();
int read_source(char *name);
void dfa();

int main(int argc, char **argv) {
    if (argc != 2) {
        puts("error: usage: minic [yout file].c");
        return 1;
    }
    if(!read_source(argv[1]))
        puts("error: file must exist");
    
    init_state_table(); 

    dfa();
    
}
