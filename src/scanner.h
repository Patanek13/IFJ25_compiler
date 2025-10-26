/**
 * @file scanner.h
 * @author Petr David Lanca
 * @brief Scanner header file for token definitions
 * @date 2025-10-01
 *
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

#define BUFFER_SIZE 512

// Token types enum prototype
typedef enum {
    // Single character tokens
    BLOCK_START, BLOCK_END,
    BRACKET_START, BRACKET_END,
    PLUS, MINUS, MULTIPLY, DIVIDE,
    DOT, COMMA,

    // One or two character tokens
    EQUAL, EQUAL_EQUAL,
    LESS, LESS_EQUAL,
    MORE, MORE_EQUAL,
    NOT, NOT_EQUAL,
    AND, OR,

    // Literals
    ID,
    GLOBAL_ID,
    NUMBER,
    STRING,

    // Keywords
    CLASS, IF, ELSE, IS, NULL_KEYWORD,
    RETURN, VAR, WHILE, IFJ, STATIC, IMPORT,
    FOR, NUM_TYPE, STRING_TYPE, NULL_TYPE,

    NEW_LINE,
    EOF_TOKEN,
    ERROR
} TokenType;

// Token structure prototype
typedef struct {
    TokenType type;
    char value[BUFFER_SIZE];
} Token;

#endif // SCANNER_H