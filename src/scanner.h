/**
 * @file scanner.h
 * @author Petr David Lanca
 * @brief Lexical analyzer (scanner) for the IFJ compiler project
 * @version 1.0
 * @date 2025-11-02
 * 
 * This header defines the lexical analyzer interface for tokenizing source code
 * in the IFJ language. The scanner converts raw character input into a stream
 * of tokens that can be processed by the parser.
 * 
 * Features:
 * - Safe buffer management with overflow protection
 * - Support for comments (single-line and multi-line)
 * - String literals with escape sequences (including hex escapes)
 * - Numeric literals (integers, floats, hex, scientific notation)
 * - All IFJ language operators and keywords
 * - Comprehensive error handling
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @def BUFFER_SIZE
 * @brief Maximum size for token buffers
 * 
 * This defines the maximum length of any single token that can be processed.
 * Tokens longer than this will be truncated but scanning will continue safely.
 */
#define BUFFER_SIZE 1024

/**
 * @enum TokenType
 * @brief Enumeration of all token types recognized by the scanner
 * 
 * This enum defines every possible token type that the scanner can produce.
 * Tokens are grouped by category for better organization and maintenance.
 */
typedef enum {
    
    // Single character tokens (punctuation and simple operators)
    BLOCK_START,        /**< '{' - Opening brace */
    BLOCK_END,          /**< '}' - Closing brace */
    BRACKET_START,      /**< '(' - Opening parenthesis */
    BRACKET_END,        /**< ')' - Closing parenthesis */
    PLUS,               /**< '+' - Addition operator */
    MINUS,              /**< '-' - Subtraction operator */
    MULTIPLY,           /**< '*' - Multiplication operator */
    DIVIDE,             /**< '/' - Division operator */
    DOT,                /**< '.' - Member access operator */
    COMMA,              /**< ',' - Comma separator */
    COLON,              /**< ':' - Type annotation separator */
    QUESTION,           /**< '?' - Ternary operator */
    NEW_LINE,           /**< '\n' - Line terminator */

    // One or two character tokens (comparison and logical operators)
    EQUAL,              /**< '=' - Assignment operator */
    EQUAL_EQUAL,        /**< '==' - Equality comparison */
    LESS,               /**< '<' - Less than comparison */
    LESS_EQUAL,         /**< '<=' - Less than or equal comparison */
    MORE,               /**< '>' - Greater than comparison */
    MORE_EQUAL,         /**< '>=' - Greater than or equal comparison */
    NOT,                /**< '!' - Logical NOT operator */
    NOT_EQUAL,          /**< '!=' - Not equal comparison */
    AND,                /**< '&&' - Logical AND operator */
    OR,                 /**< '||' - Logical OR operator */

    // Literals (user-defined values)
    ID,                 /**< Regular identifier (e.g., variable names) */
    GLOBAL_ID,          /**< Global identifier starting with '__' */
    INTEGER,            /**< Integer literal (decimal or hexadecimal) */
    FLOATING,           /**< Floating-point literal */
    STRING,             /**< String literal (single or multi-line) */
    BOOLEAN,            /**< Boolean literal (true/false) */

    // Keywords (reserved language constructs)
    CLASS,              /**< 'class' keyword */
    IF,                 /**< 'if' keyword */
    ELSE,               /**< 'else' keyword */
    IS,                 /**< 'is' keyword */
    NULL_KEYWORD,       /**< 'null' keyword */
    RETURN,             /**< 'return' keyword */
    VAR,                /**< 'var' keyword */
    WHILE,              /**< 'while' keyword */
    IFJ,                /**< 'Ifj' built-in class */
    STATIC,             /**< 'static' keyword */
    IMPORT,             /**< 'import' keyword */
    FOR,                /**< 'for' keyword */
    NUM_TYPE,           /**< 'Num' type keyword */
    STR_TYPE,           /**< 'String' type keyword */
    NULL_TYPE,          /**< 'Null' type keyword */
    BOOL_TYPE,          /**< 'Bool' type keyword */

    // Special tokens (control and error handling)
    EOF_TOKEN,          /**< End of file marker */
    ERROR               /**< Lexical error token */
} TokenType;

/**
 * @union TokenValue
 * @brief Union for storing different types of token values
 * 
 * This union allows a single token to store different types of values
 * depending on the token type, saving memory and providing type safety.
 */
typedef union {
    int integer;                    /**< Integer value for INTEGER tokens */
    double floating;                /**< Floating-point value for FLOATING tokens */
    char string[BUFFER_SIZE];       /**< String value for STRING, ID, and GLOBAL_ID tokens */
    bool boolean;                   /**< Boolean value for BOOLEAN tokens */
} TokenValue;

/**
 * @struct Token
 * @brief Complete token structure containing type and value
 * 
 * This structure represents a complete token as produced by the scanner.
 * It contains both the token type (what kind of token it is) and the
 * associated value (if applicable).
 * 
 * @var type The type of this token (from TokenType enum)
 * @var value The value associated with this token (context-dependent)
 */
typedef struct {
    TokenType type;     /**< The type of this token */
    TokenValue value;   /**< The value of this token (if applicable) */
} Token;

/**
 * @section API Function Documentation
 * 
 * The scanner API is organized into logical groups for easy navigation
 * and maintenance. Each function is documented with its purpose,
 * parameters, return values, and any important behavior notes.
 */

// === Buffer Management Functions ===
/**
 * @brief Resets the internal token buffer to empty state
 * 
 * Clears the buffer and resets the buffer index to 0. This should be
 * called before starting to scan a new token.
 */
void reset_buffer(void);

// === Character Processing Functions ===
/**
 * @brief Advances to the next character in the input stream
 * 
 * Reads the next character from the input file and adds it to the buffer
 * (if there's space). Always returns the character even if buffer is full.
 * 
 * @return The next character from input, or EOF if end of file reached
 * @note Safe against buffer overflow - will not write past buffer bounds
 */
char advance(void);

/**
 * @brief Peeks at the next character without consuming it
 * 
 * Looks ahead at the next character in the input stream without advancing
 * the stream position. Used for lookahead in parsing decisions.
 * 
 * @return The next character in the stream, or EOF if end of file
 * @note Does not modify the buffer or current position
 */
char peek(void);

/**
 * @brief Checks if next character matches expected value and consumes it
 * 
 * Tests whether the next character in the stream matches the expected
 * character. If it matches, consumes the character and returns true.
 * 
 * @param expected The character to match against
 * @return true if next character matches and was consumed, false otherwise
 */
bool match(char expected);

// === Token Creation Functions ===
/**
 * @brief Safely copies string data to a token's string value
 * 
 * Copies up to length characters from source to the token's string field,
 * ensuring null termination and preventing buffer overflow.
 * 
 * @param token Pointer to token to store string in
 * @param source Source string to copy from
 * @param length Number of characters to copy
 */
void safe_copy_to_token_string(Token* token, const char* source, size_t length);

/**
 * @brief Safely null-terminates the internal buffer
 * 
 * Adds a null terminator to the buffer at the current position,
 * with bounds checking to prevent overflow.
 */
void null_terminate_buffer(void);

/**
 * @brief Creates a token of the specified type with current buffer contents
 * 
 * Constructs a complete token from the current buffer state. Automatically
 * handles type-specific value conversion (string, integer, float).
 * 
 * @param type The type of token to create
 * @return A complete Token structure ready for use
 */
Token add_token(TokenType type);

// === Keyword and Identifier Functions ===
/**
 * @brief Determines if a word is a keyword or identifier
 * 
 * Searches the keyword table to determine if the given word is a
 * reserved keyword or should be treated as a regular identifier.
 * 
 * @param word Null-terminated string to check
 * @return Appropriate TokenType (keyword type or ID)
 */
TokenType lookup_keyword(const char* word);

// === Specialized Scanning Functions ===
/**
 * @brief Scans and validates identifier characters
 * 
 * @param ch Character to test
 * @return true if character is valid in an identifier
 */
bool is_identifier_char(char ch);

/**
 * @brief Scans a complete identifier or keyword token
 * 
 * @return Token containing the identifier or keyword
 */
Token scan_identifier(void);

/**
 * @brief Scans a global identifier (starts with __)
 * 
 * @return GLOBAL_ID token or ERROR if malformed
 */
Token scan_global_id(void);

// === Number Parsing Functions ===
/**
 * @brief Tests if character is a decimal digit
 * 
 * @param ch Character to test
 * @return true if ch is '0'-'9'
 */
bool is_digit(char ch);

/**
 * @brief Tests if character is a hexadecimal digit
 * 
 * @param ch Character to test  
 * @return true if ch is '0'-'9', 'a'-'f', or 'A'-'F'
 */
bool is_hex_digit(char ch);

/**
 * @brief Scans a sequence of decimal digits
 * 
 * @return true if at least one digit was found
 */
bool scan_digits(void);

/**
 * @brief Scans scientific notation exponent
 * 
 * @return FLOATING token if valid, ERROR if malformed
 */
Token scan_exponent(void);

/**
 * @brief Scans decimal numbers (integers and floats)
 * 
 * @return INTEGER or FLOATING token
 */
Token scan_decimal_number(void);

/**
 * @brief Scans hexadecimal numbers (0x...)
 * 
 * @return INTEGER token or ERROR if malformed
 */
Token scan_hex_number(void);

/**
 * @brief Handles numbers starting with zero
 * 
 * @return Appropriate numeric token type
 */
Token scan_zero(void);

// === String Parsing Functions ===
/**
 * @brief Safely adds a character to the current buffer
 * 
 * @param ch Character to add
 */
void add_char_to_buffer(char ch);

/**
 * @brief Processes escape sequences in strings
 * 
 * @return true if escape sequence was valid
 */
bool handle_escape_sequence(void);

/**
 * @brief Scans regular string literals
 * 
 * @return STRING token or ERROR if unterminated
 */
Token scan_regular_string(void);

/**
 * @brief Scans multi-line string literals (triple quotes)
 * 
 * @return STRING token or ERROR if unterminated
 */
Token scan_multiline_string(void);

/**
 * @brief Main string scanning dispatcher
 * 
 * @return Appropriate string token type
 */
Token scan_string(void);

// === Operator Parsing Functions ===
/**
 * @brief Scans single-character operators
 * 
 * @param op Character to process
 * @return Corresponding operator token or ERROR
 */
Token scan_single_char_operator(char op);

/**
 * @brief Scans comparison operators (may be two characters)
 * 
 * @param op First character of operator
 * @return Comparison operator token
 */
Token scan_comparison_operator(char op);

/**
 * @brief Scans logical operators (&&, ||)
 * 
 * @param op First character of operator
 * @return Logical operator token or ERROR
 */
Token scan_logical_operator(char op);

/**
 * @brief Main operator scanning dispatcher
 * 
 * @param op Character to process as operator
 * @return Appropriate operator token
 */
Token scan_operator(char op);

// === Core API Functions ===
/**
 * @brief Main tokenization function - scans next token from input
 * 
 * This is the primary interface function that clients should call to
 * get the next token from the input stream. Handles all token types
 * and error conditions automatically.
 * 
 * @return The next token from the input stream
 * @note Returns EOF_TOKEN when end of file is reached
 * @note Returns ERROR token for lexical errors
 */
Token get_token(void);

// === Utility and Debug Functions ===
/**
 * @brief Prints a token to the output stream for debugging
 * 
 * @param token Token to print
 */
void print_token(Token token);

/**
 * @brief Test function that tokenizes entire input and prints results
 * 
 * Utility function for testing and debugging the scanner.
 * Processes input until EOF and prints all tokens found.
 */
void prototype_parser_function(void);

#endif // SCANNER_H