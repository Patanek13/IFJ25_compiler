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
    NEW_LINE,

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
    EOF_TOKEN,
    ERROR
} TokenType;

/**
 * @union TokenValue
 * @brief Union for storing int, float, string, or boolean token values
 * @var integer Integer value
 */
typedef union {
    int integer;
    double floating;
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

// === Function declarations ========================================

// Buffer management functions
void reset_buffer();

// Character processing functions
char advance();
char peek();
bool match(char d);

// Token creation functions
Token add_token(TokenType type);

// Keyword and identifier functions
TokenType lookup_keyword(const char* word);

// Comment handling functions
Token single_line_comment();
Token multi_line_comment();
Token handle_slash();

// Helper functions
bool is_identifier_char(char ch);
Token scan_identifier();
Token scan_global_id();

// Number parsing functions
bool is_digit(char ch);
bool is_hex_digit(char ch);
bool scan_digits();
Token scan_exponent();
Token scan_decimal_number();
Token scan_hex_number();
Token scan_zero();

// String parsing functions
void add_char_to_buffer(char ch);
bool handle_escape_sequence();
Token scan_regular_string();
Token scan_multiline_string();
Token scan_string();

// Operator parsing functions
Token scan_single_char_operator(char op);
Token scan_comparison_operator(char op);
Token scan_logical_operator(char op);
Token scan_operator(char op);

// Main tokenizer function
Token get_token();

// Output and utility functions
void print_token(Token token);
void prototype_parser_function();

#endif // SCANNER_H