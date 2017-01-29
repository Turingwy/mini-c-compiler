#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "symbol.h"
#include "ast.h"
#include "inter.h"
#include "util.h"

label new_label() {
    static label current_label;

    return current_label++;
}

void emit_id_type(enum id_type type) {
    switch (type) {
        case id_char:
            printf("%s ", "char");
            break;
        case id_short:
            printf("%s ", "short");
            break;
        case id_int:
            printf("%s ", "int");
            break;
        case id_long:
            printf("%s ", "long");
            break;
        case id_float:
            printf("%s ", "float");
        default:
            break;
    }
}

char *emit_op(enum ast_type type) {
    switch(type) {
        case ast_type_and:
            return "&&";        
        case ast_type_equal:
            return "==";
        case ast_type_gt:
            return ">";
        case ast_type_lt:
            return "<";
        case ast_type_get:
            return ">=";
        case ast_type_let:
            return "<=";
        case ast_type_or:
            return "||";
        case ast_type_add:
            return "+";
        case ast_type_sub:
            return "-";
        case ast_type_mul:
            return "*";
        case ast_type_divi:
            return "/";
        case ast_type_nequal:
            return "!";
        default:
            return NULL;
    }
}



void emit_label(label l) {
    printf("Label%d:\n", l);
}

void emit_jmp(label l) {
    printf("jmp Label%d\n", l);
}

void emit_assign1(char *left, char *right) {
    printf("%s = %s\n", left, right);
}

void emit_assign2(char *left, char *right1, char *right2, char *op) {
    printf("%s = %s %s %s\n", left, right1, op, right2);
}

void gen_var(char *var) {
    static int var_id;
    strcpy(var, "var_");
    int_to_str(var_id++, var+4);
}

void translate_to_3addr(ast_node *node) {
    if(node == NULL)
        return;
    switch (node->type) {

        case ast_type_program:
        {
            translate_to_3addr(((ast_program *)node)->func);
            break;
        }
        case ast_type_function:
        {
            translate_to_3addr(((ast_function *)node)->block_seq);
            break;
        }
        case ast_type_seq: 
        {
            translate_to_3addr(((ast_seq *)node)->stmt);
            translate_to_3addr(((ast_seq *)node)->rest);
            break;
        }
        case ast_type_if:
        {
            ast_if *if_node = (ast_if *)node;
            translate_to_3addr(if_node->exp);
            label if_label = new_label();
            label next_label = new_label();
            label else_label = -1; 
            char *var = NULL;  
            switch (if_node->exp->type) {
                case ast_type_and:
                case ast_type_equal:
                case ast_type_gt:
                case ast_type_lt:
                case ast_type_get:
                case ast_type_let:
                case ast_type_or:
                case ast_type_add:
                case ast_type_sub:
                case ast_type_mul:
                case ast_type_divi:
                case ast_type_nequal:
                    var = ((ast_exp *)(if_node->exp))->var_name;
                    break;
                case ast_type_num:
                    var = ((ast_num *)(if_node->exp))->var_name;
                    break;
                case ast_type_self_add:
                    var = ((ast_self_op *)(if_node->exp))->var_name;
                    break;
                case ast_type_id:
                    var = ((ast_id *)(if_node->exp))->id_symbol->var_name;
                    break;
                default:
                    break;
            }

            if(if_node->else_seq != NULL) {
                else_label = new_label();
                printf("if %s!=0 jmp label%d jmp label%d\n", var, if_label, else_label);
                emit_label(if_label);
                translate_to_3addr(if_node->seq);
                emit_jmp(next_label);
                emit_label(else_label);
                translate_to_3addr(if_node->else_seq);
                emit_jmp(next_label);
            } else {
                printf("if %s!=0 jmp Label%d else jmp Label%d\n", var, if_label, next_label);
                emit_label(if_label);
                translate_to_3addr(if_node->seq);
                emit_jmp(next_label);
            }
            emit_label(next_label);
            break;
        }
        case ast_type_while:        // TODO
        case ast_type_assign:
        {
            ast_assign *assign = (ast_assign *)node;
            ast_id *id = (ast_id *)(assign->id);
            translate_to_3addr((ast_node *)id);
            translate_to_3addr(assign->exp);
            ast_node_var *node_var = (ast_node_var *)(assign->exp);
            emit_assign1(id->id_symbol->var_name, node_var->var_name);
            break;
        }
        case ast_type_or:
        case ast_type_equal:
        case ast_type_nequal:
        case ast_type_lt:
        case ast_type_gt:
        case ast_type_and:
        case ast_type_let:
        case ast_type_get:
        case ast_type_add:
        case ast_type_sub:
        case ast_type_mul:
        case ast_type_divi:
        {
            ast_exp *exp_node = (ast_exp *)node;
            translate_to_3addr(exp_node->left);
            translate_to_3addr(exp_node->right);
            gen_var(exp_node->var_name);
            char *left_var, *right_var;
            if(exp_node->left->type == ast_type_id)
                left_var = ((ast_id *)(exp_node->left))->id_symbol->var_name;
            else
                left_var = ((ast_node_var *)(exp_node->left))->var_name;

            if(exp_node->right->type == ast_type_id)
                right_var = ((ast_id *)(exp_node->right))->id_symbol->var_name;
            else
                right_var = ((ast_node_var *)(exp_node->right))->var_name;

            emit_assign2(exp_node->var_name, left_var, right_var, emit_op(exp_node->type));
            break;
        }

        case ast_type_self_add:
        case ast_type_self_sub:
        {
            ast_self_op *self_node = (ast_self_op *)node;
            if(self_node->id->id_symbol->var_name[0] == '\0') {
                gen_var(self_node->id->id_symbol->var_name);
            }
            char *s = NULL;
            if(node->type == ast_type_self_add)
                s = "+";
            else
                s = "-";
            emit_assign2(self_node->id->id_symbol->var_name, self_node->id->id_symbol->var_name, 
                    "1", s);
            strcpy(self_node->var_name, self_node->id->id_symbol->var_name);
            break;
        }
        case ast_type_num: 
        {
            ast_num *num_node = (ast_num *)node;
            gen_var(num_node->var_name);
            emit_assign1(num_node->var_name, num_node->value);
            break;
        }
        case ast_type_id:
        {
            ast_id *id_node = (ast_id *)node;
            if(id_node->id_symbol->var_name[0] == '\0')
                gen_var(id_node->id_symbol->var_name);
            break;
        }

    }
}


