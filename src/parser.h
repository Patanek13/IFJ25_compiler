/**
 * @file parser.h
 * @author Šimon Čorej xcorejs00
 * @brief Parser header file
 * @date 2025-10-28
 * 
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "scanner.h"
#include "error.h"

#define MAX_SIZE 150
#define STACK_SIZE 30
#define ROW 14
#define COL 14

/*============================== Scanner function declaration ==============================*/
Token get_token();
void scanner_init(FILE* source, FILE* output);

/*============================== Token and Stack declaration ===============================*/
Token token;

typedef struct{
    TokenType* array;
    int topIndex;
}Stack;

/*============================== Parser function declaration ===============================*/
int cond_loop();
int command();
int assign();
int block();
int built_in_call();
int func_decl();
int func_call();
int params();
int valid();
int program();
int expression_val(Stack* stack);

/*=================================== Precedence Table =====================================*/


/**
 * @brief 2D arrays of {<, >, =, ERROR} TokenTypes [ROW][COL]
 * @details ROW = TokenType on top of stack
 * @details COL = TokenType given by scanner
 */
/*    +     -     /     *      (    )     ID    GID   VAR   STR   INT  FLO   BOOL    $ */
TokenType precedence_table[ROW][COL] = {
    {MORE, MORE, LESS, LESS, LESS, MORE, LESS, LESS, LESS, LESS, LESS, LESS, LESS, MORE},           /* + */
    {MORE, MORE, LESS, LESS, LESS, MORE, LESS, LESS, LESS, LESS, LESS, LESS, LESS, MORE},           /* - */
    {MORE, MORE, MORE, MORE, LESS, MORE, LESS, LESS, LESS, LESS, LESS, LESS, LESS, MORE},           /* / */
    {MORE, MORE, MORE, MORE, LESS, MORE, LESS, LESS, LESS, LESS, LESS, LESS, LESS, MORE},           /* * */       
    {LESS, LESS, LESS, LESS, LESS, EQUAL, LESS, LESS, LESS, LESS, LESS, LESS, LESS, ERROR},         /* ( */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* ) */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* ID */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* GID */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* VAR */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* STR */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* INT */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* FLO */
    {MORE, MORE, MORE, MORE, ERROR, MORE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, MORE},   /* BOOL */
    {LESS, LESS, LESS, LESS, LESS, ERROR, LESS, LESS, LESS, LESS, LESS, LESS, LESS, ERROR}          /* $ */
};

/*=================================== Precedence table functions ===========================*/

/**
 * @brief Translates TokenTypes used in precedence table to integers
 * 
 * @param in_token Token whose type we want to translate 
 * @return Int or Error 
 */
int token_to_int(Token in_token){
    switch(in_token.type){
        case OPERATOR:
            if (strcmp(in_token.value.string, "+") == 0){ return 0; }
            if (strcmp(in_token.value.string, "-") == 0){ return 1; }
            if (strcmp(in_token.value.string, "/") == 0){ return 2; }
            if (strcmp(in_token.value.string, "*") == 0){ return 3; }

            break;

        case BRACKET_START: return 4;
        case BRACKET_END:   return 5;
        case ID:            return 6;
        case GLOBAL_ID:     return 7;
        case VAR:           return 8;
        case STRING:        return 9;
        case INTEGER:       return 10;
        case FLOATING:      return 11;
        case BOOLEAN:       return 12;

        default: 
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}


/*=================================== Expression  calculation function =====================*/

int expression_val(Stack* stack){ /* neskor doplnit relacne negacne a ternarne operatory */
    switch(token.type){
        case OPERATOR:
        case BRACKET_START:
        case BRACKET_END:
        case ID:
        case GLOBAL_ID:
        case VAR:
        case STRING:
        case INTEGER:
        case FLOATING:
        case BOOLEAN:

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;    
}

/*=================================== Stack Functions ======================================*/
/**
 * @brief Initializes stack and allocates memmory for elements
 * 
 * @param stack Initialized stack
 * @return ERR_OK or SYNTAX_ERROR
 */
int stack_init(Stack* stack){

    if (stack == NULL){
        return SYNTAX_ERROR;
	}

	TokenType* tmp = (TokenType *)malloc(STACK_SIZE * sizeof(TokenType));

	if (tmp == NULL){
		return SYNTAX_ERROR;
	}
	stack->array = tmp;
	stack->topIndex = -1;

    return ERR_OK;
}

/**
 * @brief Checks if stack is full
 * 
 * @param stack Initialized stack
 * @return true if topIndex is one below the STACK_SIZE (maximum) thus full
 * @return false if not
 */
bool stack_is_full(const Stack* stack){
    return (stack->topIndex == (STACK_SIZE - 1));
}

/**
 * @brief Checks if stack is empty
 * 
 * @param stack Initialized stack
 * @return true if topIndex is -1
 * @return false if not
 */
bool stack_is_empty(const Stack *stack) {
	return stack->topIndex == -1;
}

/**
 * @brief Deletes TokenType located on top of the stack, if stack is empty does nothing
 * 
 * @param stack Initialized stack
 * @return ERR_OK or SYNTAX_ERROR
 */
int stack_pop(Stack* stack){
    if (!stack_is_empty(stack)){
        stack->topIndex--;
        return ERR_OK;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Assigns TokenType located on top of the stack to pointer given by parameter, if stack is empty does nothing
 * 
 * @param stack Initialized stack
 * @param token_ptr Pointer to which we assign value
 */
void stack_top(Stack* stack, TokenType* token_ptr){
    if (!stack_is_empty(stack)){
        *token_ptr = stack->array[stack->topIndex];
    }
}

/**
 * @brief Pushes TokenType given by parameter to top of the stack, if stack is full returns error
 * 
 * @param stack Initialized stack
 * @param token TokenType which we want to push
 * @return ERR_OK or SYNTAX_ERROR 
 */
int stack_push(Stack* stack, TokenType token){
    if (stack_is_full(stack)){
		return SYNTAX_ERROR;
	}
    stack->array[++stack->topIndex] = token;
    return ERR_OK;
}

/**
 * @brief Frees array of TokenTypes, sets pointer array to NULL and resets topIndex
 * 
 * @param stack Initialized stack
 */
void stack_destroy(Stack* stack){
    free(stack->array);
    stack->array = NULL;
    stack->topIndex = -1;
}


#endif