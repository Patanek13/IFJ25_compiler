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


Token get_token();
void print_token(Token token);

typedef struct{
    TokenType type;
    bool terminal;
} StackElement;

typedef struct{
    StackElement* element;
    int topIndex;
}Stack;

bool is_operator();


#endif