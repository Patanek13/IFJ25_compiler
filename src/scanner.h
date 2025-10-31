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
#include <stdbool.h>

#define BUFFER_SIZE 1024

/**
 * @enum TokenType
 * @brief Token types recognized by the scanner
 */
typedef enum {
    
    // Single character tokens
    BLOCK_START, BLOCK_END,
    BRACKET_START, BRACKET_END,
    PLUS, MINUS, MULTIPLY, DIVIDE,
    DOT, COMMA, COLON, QUESTION,

    // One or two character tokens
    EQUAL, EQUAL_EQUAL,
    LESS, LESS_EQUAL,
    MORE, MORE_EQUAL,
    NOT, NOT_EQUAL,
    AND, OR,

    // Literals
    ID,
    GLOBAL_ID,
    INTEGER,
    FLOATING,
    STRING,
    BOOLEAN,

    // Keywords
    CLASS, IF, ELSE, IS, NULL_KEYWORD,
    RETURN, VAR, WHILE, IFJ, STATIC, IMPORT, FOR,
    NUM_TYPE, STR_TYPE, NULL_TYPE, BOOL_TYPE,

    // Special tokens
    NEW_LINE,
    EOF_TOKEN,
    ERROR
} TokenType;

/**
 * @union TokenValue
 * @brief Union for storing int, float, string, or boolean token values
 * 
 */
typedef union {
    int integer;
    float floating;
    char string[BUFFER_SIZE];
    bool boolean;
} TokenValue;

/**
 * @struct Token
 * @brief Token structure for storing token type and value
 * 
 */
typedef struct {
    TokenType type;
    TokenValue value;
} Token;

#endif // SCANNER_H