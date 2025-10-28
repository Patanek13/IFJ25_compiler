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

Token token;

// void stack_init(Stack* stack){
//     stack->element = (StackElement*)malloc(MAX_SIZE * sizeof(StackElement)); /* PORIADNE PREMAZAT */
//     if (!stack->element){ /* error */}
//     stack->topIndex = -1;
// }

// void stack_pop(Stack* stack){
//     stack->topIndex--;
// }
// void stack_push_token(Stack* stack, TokenType type){
//     stack->element[++stack->topIndex].state = NULL_NAME;
//     stack->element[stack->topIndex].type = type;
//     stack->element[stack->topIndex].terminal = true;    
// }

// void stack_push_state(Stack* stack, STATES state){
//     stack->element[++stack->topIndex].state = state;
//     stack->element[stack->topIndex].type;
//     stack->element[stack->topIndex].terminal = false;
// }

// bool is_operator(Token token){ /* doplnit!!!*/
//     return ((token.type == MULTIPLY) || (token.type == PLUS) || (token.type == MINUS) || (token.type == DIVIDE));
// }

// bool is_operand(Token token){
//     return((token.type == ID) || (token.type == GLOBAL_ID) || (token.type == NUMBER) || (token.type == STRING)); /* plus func return*/
// }

// int command_push(Stack* stack){ /* nedokoncene pozor na nacitanie tokenu! tu by mal byt iba vyber state a automaticke popnutie stacku*/
//     Token token;
//     token = get_token();

//     switch(token.type){
//         case IFJ:
//             stack_rule_push(BUILT_IN, stack);
//             return 0;
//         case ID:
//             token = get_token();
//             if (token.type == DOT){
//                 stack_rule_push(FUNC_CALL, stack);
//                 return 1;
//             } else if (is_operator(token)){
//                 stack_rule_push(EXPRESSION, stack);
//                 return 1;
//             }
//             return -1;
//         case GLOBAL_ID:
//         case VAR:
//             stack_rule_push(DECLARE, stack);
//             return 2;
//         case RETURN:
//             stack_rule_push(RETURN_VAL, stack);
//             return 3;

//         default:
//             return -1;
//     }
// }


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
// int stack_rule_push(){ /* poupravit aby sa tokeny spravne porovnavali kedze sa tu znova ziskavaju */
//     switch(token.type){
//         case VALID: /* <valid> -> IMPORT "ifj25" FOR IFJ \n <program>*/
//             stack_push_state(stack, PROGRAM);
//             stack_push_token(stack, NEW_LINE);
//             stack_push_token(stack, IFJ);
//             stack_push_token(stack, FOR);
//             stack_push_token(stack, STRING); /*ifj25*/
//             stack_push_token(stack, IMPORT);
//             return 1;
//         case PROGRAM:
//             stack_push_state(stack, BLOCK);
//             stack_push_token(stack, ID); /* Program*/
//             stack_push_token(stack, CLASS);
//             return 1;
//         case BLOCK:
//             stack_push_token(stack, BLOCK_END);
//             stack_push_state(stack, COMMAND_N);
//             stack_push_state(stack, COMMAND);
//             stack_push_token(stack, NEW_LINE);
//             stack_push_token(stack, BLOCK_START);
//             return 1;
//         case COMMAND: /* tu pozor na chybajuce command_n*/
//             if (command_push(stack) < 0){
//                 printf("SYNTAX ERROR\n");
//                 return -1;
//             }
//             stack_push_token(stack, NEW_LINE);
//             return 1;
//         case COMMAND_N: /* este doplnit to aby command-n nemusel existovat*/
//             stack_push_token(stack, NEW_LINE);
//             stack_rule_push(COMMAND, stack);
//             return 1;
//         case FUNCTION:
//             stack_push_state(stack, BLOCK);
//             stack_push_state(stack, BRACKETS);
//             stack_push_token(stack, ID);
//             stack_push_token(stack, STATIC);
//             return 1;
        
//         case FUNCTION_SETTER:
//             stack_push_state(stack, BLOCK);
//             stack_push_token(stack, BRACKET_END);
//             stack_push_state(stack, PARAMS);
//             stack_push_token(stack, BRACKET_START);
//             stack_push_token(stack, ID);
//             stack_push_token(stack, STATIC);
//             return 1;
        
//         case FUNC_CALL:
//             stack_push_token(stack, NEW_LINE);
//             stack_push_token(stack, BRACKET_END);
//             stack_push_state(stack, PARAMS_N);
//             stack_push_state(stack, PARAMS);
//             stack_push_token(stack, BRACKET_START);
//             stack_push_token(stack, ID);
//             return 1;

//         case BUILT_IN:
//             stack_push_state(stack, COMMAND_N);
//             stack_push_token(stack, BRACKET_END);
//             stack_push_state(stack, PARAMS_N);
//             stack_push_state(stack, PARAMS);
//             stack_push_token(stack, BRACKET_START);
//             stack_push_token(stack, ID);
//             stack_push_token(stack, DOT);
//             /* pripadne aj IFJ*/
//             return 1;

//         case COND_LOOP:
//             stack_push_state(stack, COND_LOOP_N);
//             stack_push_state(stack, BLOCK);
//             stack_push_token(stack, BRACKET_END);
//             stack_push_state(stack, EXPRESSION);
//             stack_push_state(stack, IF); /* alebo while*/
//             return 1;
        
//         case COND_LOOP_N:
//             stack_push_state(stack, BLOCK);
//             stack_push_token(stack, ELSE);
//             return 1;

//         case PARAMS: /* este doplnit to aby nemusel existovat a bool ()*/ 
//             Token token;
//             token = get_token();
//             if ((token.type != ID) && (token.type != NUMBER) && (token.type != STRING)){
//                 printf("SYNTAX ERROR\n");
//                 return -1;
//             }
//             return 1;

//         case PARAMS_N: /* este doplnit to aby nemusel existovat (var)*/
//             stack_push_state(stack, PARAMS_N);
//             stack_push_state(stack, PARAMS);
//             stack_push_token(stack, COMMA);
//             return 1;
        
//         case EXPRESSION: /* POZOR toto implementovat pomocou prediktivnej syntaktickej analyzy*/
//             /* call PSA */
//             // stack_push_state(stack, EXPRESSION_N, false);
//             // stack_push_state(stack, OPERAND, false);
//             // stack_push_state(stack, OPERATOR, false);
//             // stack_push_state(stack, OPERAND, false);
//             return 1;
        
//         case EXPRESSION_N:
//             /* call PSA */
//             // stack_push_state(stack, EXPRESSION_N, false);
//             // stack_push_state(stack, OPERAND, false);
//             // stack_push_state(stack, OPERATOR, false);
//             return 1;
        
//         case OPERAND:
//             if (is_operand(token)){
//                 return 1;
//             }else{
//                 return -1;
//             }

//         case OPERATOR:
//             if (is_operator(token)){
//                 return 1;
//             } else {
//                 return -1;
//             }
        
            
//         default: 
//             return -1;

//     }
//     return -1;
// }

bool match(TokenType type){
    token = get_token();
    return (token.type == type);
}

int func_call(){
    switch(token.type){
        case ID:
            if (!match(BRACKET_START)){ return SYNTAX_ERROR; }
            return func_call();
        break;

        case BRACKET_START:
            if (match(BRACKET_END)){ 
                return func_call();
            } else {
                return params();
            }
        break;
        case PARAMS:
            /* chod do params switchu*/
        case BRACKET_END:
            token = get_token();
            if (token.type != NEW_LINE){ return SYNTAX_ERROR; }
            return ERR_OK;
        break;

        return SYNTAX_ERROR;
    }
}



int command(){
   switch(token.type){
        case ID:
            token = get_token();
            if (token.type == BRACKET_START){ 
                return func_call();
            } else if (is_operator(token.type)) {
                return expression();
            } else {
                return SYNTAX_ERROR;
            }
        break;

        case IFJ:
            return built_in_call();
        break;

        case RETURN:
            token = get_token();
            if ((token.type != ID) && (token.type != GLOBAL_ID) && (token.type != NUMBER) 
                && (token.type != STRING) && (token.type != BOOLEAN)){ return SYNTAX_ERROR; }
        break;

        case GLOBAL_ID:
        case VAR:
        case IF:
        case STATIC: 

   }
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