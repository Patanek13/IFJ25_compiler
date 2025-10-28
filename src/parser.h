/**
 * @file parser.h
 * @author Šimon Čorej xcorejs00
 * @brief Parser header fiel for definitions of stack functions and states
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


// FILE *file;
// FILE *output_file;

Token get_token();
void print_token(Token token);

typedef enum{
    NULL_NAME, /* to push key words or necessary strings TEMPORARY!!!*/
    VALID, /* import ifj check*/
    PROGRAM, /* start of program*/
    FUNCTION, /* static func declare*/
    FUNCTION_SETTER,
    COMMAND, /* individual commands in blocks*/
    COMMAND_N, /* allows additional comands*/
    FUNC_CALL, /* call of declared user made function*/
    RETURN_VAL,
    PARAMS, 
    PARAMS_N,
    BLOCK, /* block of commands */
    EXPRESSION, /* eval, assigning values, setter value, ... */
    EXPRESSION_N,
    OPERATOR, 
    OPERAND,
    DECLARE, /* declaration of global or local variables*/
    COND_LOOP, /* if or while functions*/
    COND_LOOP_N, /* else loop only possible for if*/
    BUILT_IN, /* call of built in functions Ifj. ...*/
    BRACKETS, /* additional (<params> <params-n>) rule to differentiate setters getters and user made functions*/
}STATES;

/**
 * @brief Struct StackElement stored on stack
 * 
 * STATES state = list of possible states or characters that can occur
 * char* value = value of possible state or character
 * bool terminal = instructs either given state is terminal or not
 */

typedef struct{
    STATES state;
    TokenType type;
    // char* value;
    bool terminal;
} StackElement;

typedef struct{
    StackElement* element;
    int topIndex;
}Stack;




#endif