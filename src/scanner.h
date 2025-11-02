/**
 * @file scanner.h
 * @author Petr David Lanca
 * @brief Scanner header file
 * @date 2.11.2025
 * 
 * Buffer size
 * 
 * Token structure
 * - Token types enum
 * - Token value union
 * 
 * Function prototypes
 * 
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @def BUFFER_SIZE
 * @brief Amount of saved characters while scanning a single token
 * 
 */
#define BUFFER_SIZE 1024

/**
 * @enum TokenType
 * @brief Enum for all token types recognized by the scanner
 * 
 */

typedef enum {
    
    BLOCK_START,        /**< { */
    BLOCK_END,          /**< } */
    BRACKET_START,      /**< ( */
    BRACKET_END,        /**< ) */
    PLUS,               /**< + */
    MINUS,              /**< - */
    MULTIPLY,           /**< * */
    DIVIDE,             /**< / */
    DOT,                /**< . */
    COMMA,              /**< , */
    COLON,              /**< : */
    QUESTION,           /**< ? */
    NEW_LINE,           /**< \n */

    EQUAL,              /**< = */
    EQUAL_EQUAL,        /**< == */
    LESS,               /**< < */
    LESS_EQUAL,         /**< <= */
    MORE,               /**< > */
    MORE_EQUAL,         /**< >= */
    NOT,                /**< ! */
    NOT_EQUAL,          /**< != */
    AND,                /**< && */
    OR,                 /**< || */

    ID,                 /**< Identifier - variable and function names */
    GLOBAL_ID,          /**< Global identifier - starts with '__' */
    INTEGER,            /**< Integer - normal, hexadecimal */
    FLOATING,           /**< Floating - decimal, exponential */
    STRING,             /**< String literal - normal and multi-line */
    BOOLEAN,            /**< Boolean literal - true or false */

    CLASS,              /**< class */
    IF,                 /**< if */
    ELSE,               /**< else */
    IS,                 /**< is */
    NULL_KEYWORD,       /**< null */
    RETURN,             /**< return */
    VAR,                /**< var */
    WHILE,              /**< while */
    IFJ,                /**< Ifj */
    STATIC,             /**< static */
    IMPORT,             /**< import */
    FOR,                /**< for */
    NUM_TYPE,           /**< Num */
    STR_TYPE,           /**< String */
    NULL_TYPE,          /**< Null */
    BOOL_TYPE,          /**< Bool */

    EOF_TOKEN,          /**< End of file */
    ERROR               /**< Lexical error */

} TokenType;

/**
 * @union TokenValue
 * @brief Union for storing different types of token values
 * 
 */
typedef union {
    int integer;                    /**< INTEGER token value */
    double floating;                /**< FLOATING token value */
    char string[BUFFER_SIZE];       /**< STRING, ID, GLOBAL_ID token value */
    bool boolean;                   /**< BOOLEAN token value */
} TokenValue;

/**
 * @struct Token
 * @brief Token structure to store type and value
 * 
 * @var Token type
 * @var Token value (optional)
 */
typedef struct {
    TokenType type;     /**< Token type */
    TokenValue value;   /**< Token value (optional) */
} Token;

//================================================================================================
//                                      HELPER FUNCTIONS
//================================================================================================

//------------------------------------- Buffer ---------------------------------------------------

/**
 * @brief Resets the token buffer
 * 
 * Sets every character to null and resets index to 0
 */
void reset_buffer(void);

//------------------------------------- Input ----------------------------------------------------

/**
 * @brief Get next character and save it to buffer
 * 
 * If buffer is full, character is not saved to buffer
 * 
 * @return Next character
 */
char advance(void);

/**
 * @brief Read next character and put it back to input stream
 * 
 * @return Next character
 */
char peek(void);

/**
 * @brief Compare character with next character
 * 
 * If characters match, save the character to buffer
 * 
 * @param expected Character to check
 * @return true if next characters match, false otherwise
 */
bool match(char expected);

//------------------------------------- Token creation -------------------------------------------

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

//================================================================================================
//                                      SCANNER STATES
//================================================================================================

/**
 * @brief Processes single-line comments starting with //
 * 
 * @return Next token after the comment
 */
Token single_line_comment(void);

/**
 * @brief Processes multi-line comments
 * 
 * @return Next token after the comment, or ERROR if unterminated
 */
Token multi_line_comment(void);

/**
 * @brief Handles the '/' character which could be division or start of comment
 * 
 * @return DIVIDE token or result of comment processing
 */
Token handle_slash(void);

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