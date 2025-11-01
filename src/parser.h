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

/*=============== Scanner function declaration ===============*/
Token get_token();
void print_token(Token token);
void scanner_innit(FILE* source, FILE* output);

/*=============== Parser function declaration ================*/
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


typedef struct{
    TokenType type;
    char value;
} StackElement;

typedef struct{
    StackElement* element;
    int topIndex;
}Stack;

bool is_operator();


#endif