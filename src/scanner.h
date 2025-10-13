#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

#define BUFFER_SIZE 255
#define KEYWORD_LIST_LENGTH 15

// Token types enum prototype
typedef enum {
    BLOCK_START,
    BLOCK_END,
    BRACKET_START,
    BRACKET_END,
    EQUAL,
    EQUAL_EQUAL,
    LESS,
    LESS_EQUAL,
    MORE,
    MORE_EQUAL,
    NOT,
    NOT_EQUAL,
    AND,
    OR,

    DOT,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,

    ID,
    KEYWORD,
    GLOBAL_ID,
    NUMBER,
    STRING,

    EOF_TOKEN
} TokenType;

// Token structure prototype
typedef struct {
    TokenType type;
    char value[BUFFER_SIZE];
} Token;

#endif // SCANNER_H