/**
 * @file parser.c
 * @author Šimon Čorej (xcorejs00)
 * @brief Main Parser Function
 * @version 0.1
 * @date 2025-10-12 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"

FILE* src;
FILE* out;

void stack_init(Stack* stack){
    stack->element = (StackElement*)malloc(MAX_SIZE * sizeof(StackElement)); /* PORIADNE PREMAZAT */
    if (!stack->element){ /* error */}
    // stack->element->value = (char*)malloc(MAX_SIZE * sizeof(char));
    stack->topIndex = -1;
}

void stack_pop(Stack* stack){
    stack->topIndex--;
}

void stack_push_token(Stack* stack, TokenType type, char* value, bool terminal){
    stack->element[++stack->topIndex].state = NULL_NAME;
    stack->element[stack->topIndex].type = type;
    stack->element[stack->topIndex].value = value;
    stack->element[stack->topIndex].terminal = terminal;    
}

void stack_push_state(Stack* stack, STATES state, bool terminal){
    stack->element[++stack->topIndex].state = state;
    stack->element[stack->topIndex].type;
    stack->element[stack->topIndex].value = '\0';
    stack->element[stack->topIndex].terminal = terminal;
}

int command_push(Stack* stack){ /* nedokncene !*/
    Token token;
    token = get_token();

    TokenType token_id_switch = token.type;

    switch(token_id_switch){
        case IFJ:
            stack_rule_push(BUILT_IN, stack);
            return 0;
        case ID:
            stack_rule_push(FUNC_CALL, stack);
            return 1;
        case GLOBAL_ID:
        case VAR:
            stack_rule_push(DECLARE, stack);
            return 2;
        case RETURN:
            stack_rule_push(RETURN_VAL, stack);
            return 3;

        default:
            return -1;
    }
}


bool stack_is_empty(Stack* stack){
    return (stack->topIndex == -1);
}

void stack_clear(Stack* stack){
    while (stack->topIndex >= 0){
        stack_pop(stack);
    }
    stack_pop(stack);
    free(stack->element);
}

/**
 * @brief Function for pushing states to stack
 * 
 * @param state List of states possible (Program state, command state, etc...)
 * @param stack Initialied stack on to which elements ar pushed
 * @return int 
 */
int stack_rule_push(STATES state, Stack* stack){ /* poupravit aby sa tokeny spravne porovnavali kedze sa tu znova ziskavaju */
    switch(state){
        case VALID:
            stack_push_state(stack, PROGRAM, false);
            stack_push_token(stack, NEW_LINE, "\n", true);
            stack_push_token(stack, ID, "ifj", true);
            stack_push_token(stack, ID, "for", true);
            stack_push_token(stack, STRING, "ifj25", true);
            stack_push_token(stack, IMPORT, "import", true);
            return 0;
        case PROGRAM:
            stack_push_state(stack, BLOCK, false);
            stack_push_token(stack, ID, "Program", true);
            stack_push_token(stack, ID, "class", true);
            return 1;
        case BLOCK:
            stack_push_token(stack, BLOCK_END, "}", true);
            stack_push_state(stack, COMMAND_N, false);
            stack_push_state(stack, COMMAND, false);
            stack_push_token(stack, NEW_LINE, "\n", true);
            stack_push_token(stack, BLOCK_START, "{", true);
            return 2;
        case COMMAND:
            if (command_push(stack) < 0){
                printf("SYNTAX ERROR\n");
                return -1;
            }
            stack_push_token(stack, NEW_LINE, "newline", true);
            return 3;
        case COMMAND_N: /* este doplnit to aby command-n nemusel existovat*/
            stack_push_token(stack, NEW_LINE, "newline", true);
            stack_rule_push(COMMAND, stack);
            return 4;
        case FUNCTION:
            stack_push_state(stack, BLOCK, false);
            stack_push_state(stack, BRACKETS, false);
            stack_push_token(stack, ID, "\0", true);
            stack_push_token(stack, STATIC, "static", true);
            return 5;
        
        case FUNCTION_SETTER:
            stack_push_state(stack, BLOCK, false);
            stack_push_token(stack, BRACKET_END, ")", true);
            stack_push_state(stack, PARAMS, false);
            stack_push_token(stack, BRACKET_START, "(", true);
            stack_push_token(stack, ID, "\0", true);
            stack_push_token(stack, STATIC, "static", true);
            return 6;

        case PARAMS: /* este doplnit to aby nemusel existovat a bool ()*/ 
            Token token;
            token = get_token();
            if ((token.type != ID) && (token.type != NUMBER) && (token.type != STRING)){
                printf("SYNTAX ERROR\n");
                return -1;
            }
            return 7;

        case PARAMS_N: /* este doplnit to aby nemusel existovat (var)*/
            stack_push_state(stack, PARAMS_N, false);
            stack_push_state(stack, PARAMS, false);
            stack_push_token(stack, COMMA, ",", true);
            return 8;
        
        case EXPRESSION:
            stack_push_state(stack, EXPRESSION_N, false);
            stack_push_state(stack, OPERAND, false);
            stack_push_state(stack, OPERATOR, false);
            stack_push_state(stack, OPERAND, false);
            return 9;
        
        
            
        default: 
            return -1;

    }
    return -1;
}



int recursion(Stack* stack, STATES state, bool finished){ /* treba prerobit nesuhlasi zo switch mozne chyby POZOR!*/
    /* tu este mozno pop treba*/
    if (!finished){
        if (stack_rule_push(state, stack) < 0){
            finished = true;
            return -1;
        }
        while(stack->element[stack->topIndex].terminal != false){

            Token token;
            token = get_token();
            print_token(token);

            fprintf(out, "token got and type is: ");
            print_token(token);
            fprintf(out, "\n");

            if (token.type == stack->element[stack->topIndex].type){
                stack_pop(stack);
            } else{
                finished = true;
                return -1;
            }
        }
        if (!stack_is_empty(stack)){
            recursion(stack, stack->element[stack->topIndex].state, false);
        }
        else{
            finished = true;
        }
    }
    return 1;
}


int tryout_func(){

    Stack stack;
    stack_init(&stack); 
    
    recursion(&stack, VALID, false);

    for (int i = stack.topIndex; i > 0; i--){
        printf("I: %d | toto je na stacku: \n stack.state: %d | stack.type: %d | terminal? %s\n_________________\n", i, stack.element[i].state, stack.element[i].type, (stack.element[i].terminal == false) ? "no":"yes");
    }

    stack_clear(&stack);
    return 0;
}



int main (int argc, char** argv){
    (void)argc;
    (void)argv;

    src = fopen("ahoj.IFJcode2025", "r");
    if (!src) {return 1;}

    out = fopen("tokens.txt", "w");
    if (!out) {return 1;}

    scanner_init(src, out);

    tryout_func();

    fclose(src);
    fclose(out);
     

      
    return 0;
}