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
 * Lookup table
 * - keyword entry struct
 * - keyword table
 *
 * Function declarations
 *
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

//================================================================================================
//                                      DEFINITIONS
//================================================================================================

/**
 * @def BUFFER_SIZE
 * @brief Amount of saved characters while scanning a single token
 *
 */
#define BUFFER_SIZE 1024

//================================================================================================
//                                      TOKEN
//================================================================================================

/**
 * @enum TokenType
 * @brief Enum for all token types recognized by the scanner
 *
 */

typedef enum {

  BLOCK_START,   /**< { */
  BLOCK_END,     /**< } */
  BRACKET_START, /**< ( */
  BRACKET_END,   /**< ) */
  PLUS,          /**< + */
  MINUS,         /**< - */
  MULTIPLY,      /**< * */
  DIVIDE,        /**< / */
  DOT,           /**< . */
  COMMA,         /**< , */
  COLON,         /**< : */
  QUESTION,      /**< ? */
  NEW_LINE,      /**< \n */

  EQUAL,       /**< = */
  EQUAL_EQUAL, /**< == */
  LESS,        /**< < */
  LESS_EQUAL,  /**< <= */
  MORE,        /**< > */
  MORE_EQUAL,  /**< >= */
  NOT,         /**< ! */
  NOT_EQUAL,   /**< != */
  AND,         /**< && */
  OR,          /**< || */

  ID,        /**< Identifier - variable and function names */
  GLOBAL_ID, /**< Global identifier - starts with '__' */
  INTEGER,   /**< Integer - normal, hexadecimal */
  FLOATING,  /**< Floating - decimal, exponential */
  STRING,    /**< String literal - normal and multi-line */
  BOOLEAN,   /**< Boolean literal - true or false */

  OPERATOR,    /**< Operators +, -, *, / for precedence parsing */
  PSEUDO_E,    /**< Pseudo-token E for expression parsing */
  MARKER_LESS, /**< Marker '<' for precedence parsing */

  CLASS,        /**< class */
  IF,           /**< if */
  ELSE,         /**< else */
  IS,           /**< is */
  NULL_KEYWORD, /**< null */
  RETURN,       /**< return */
  VAR,          /**< var */
  WHILE,        /**< while */
  IFJ,          /**< Ifj */
  STATIC,       /**< static */
  IMPORT,       /**< import */
  FOR,          /**< for */
  NUM_TYPE,     /**< Num */
  STR_TYPE,     /**< String */
  NULL_TYPE,    /**< Null */
  BOOL_TYPE,    /**< Bool */

  END_EXPR, /**< $ - end of expression */

  EOF_TOKEN, /**< End of file */
  ERROR,     /**< Lexical error */

  // Special tokens for precedence parsing
  PREC_ERR,    /**< 'X' - Error */
  PREC_SHIFT,  /**< '<' - Shift */
  PREC_REDUCE, /**< '>' - Reduce */
  PREC_EQUAL,  /**< '=' - Handle (for brackets) */

} TokenType;

/**
 * @union TokenValue
 * @brief Union for storing different types of token values
 *
 */
typedef union {
  int integer;              /**< INTEGER token value */
  double floating;          /**< FLOATING token value */
  char string[BUFFER_SIZE]; /**< STRING, ID, GLOBAL_ID token value */
  bool boolean;             /**< BOOLEAN token value */
} TokenValue;

/**
 * @struct Token
 * @brief Token structure to store type and value
 *
 * @var Token type
 * @var Token value (optional)
 */
typedef struct {
  TokenType type;   /**< Token type */
  TokenValue value; /**< Token value (optional) */
} Token;

//================================================================================================
//                                      KEYWORD LOOKUP TABLE
//================================================================================================

/**
 * @struct KeywordEntry
 * @brief Keyword entry to pair each keyword with token type
 *
 * @var char* keyword
 * @var TokenType type
 */
typedef struct {
  char *keyword;
  TokenType type;
} KeywordEntry;

//================================================================================================
//                                      FUNCTION PROTOTYPES
//================================================================================================

//------------------------------------- Initialization -------------------------------------------
/**
 * @brief Initializes the scanner with the input file
 *
 * @param input_file File pointer to the input source code
 * @param output_file File pointer to the output (error) stream
 */
void scanner_init(FILE *input_file, FILE *output_file);

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

/**
 * @brief Replace last char in buffer
 *
 * @param ch Character
 */
void replace(char ch);

//------------------------------------- Token creation -------------------------------------------

/**
 * @brief Create a token with type and value
 *
 * Convert string in buffer to value based on token type
 *
 * @param type Token type
 * @return Token with TokenType and TokenValue
 */
Token add_token(TokenType type);

//------------------------------------- Keyword lookup table -------------------------------------

/**
 * @brief Check the string if it is a keyword
 *
 * If it does not match any keyword, return ID
 *
 * @param word String to check
 * @return TokenType of correct keyword or ID
 */
TokenType lookup_keyword(const char *word);

//================================================================================================
//                                      SCANNER STATES
//================================================================================================

//------------------------------------- Comments -------------------------------------------------

/**
 * @brief Loop until end of line
 *
 * Recursively calls for the next token
 *
 * @return Next token after the comment
 */
Token single_line_comment(void);

/**
 * @brief Loop until end of multi-line comment
 *
 * Recursively call for the next token
 *
 * @return Next token after the comment, ERROR if unterminated
 */
Token multi_line_comment(void);

/**
 * @brief Decide if '/' is DIVIDE token, start of Comment or Multiline comment
 *
 * Called when 1st char of new token is '/'
 *
 * @return Divide token (optional)
 */
Token scan_slash(void);

//------------------------------------- IDs and keywords -----------------------------------------

/**
 * @brief Scan identifier or keyword
 *
 * @return TokenType ID or correct keyword
 */
Token scan_identifier(void);

/**
 * @brief Scan global identifier
 *
 * Global ID starts with '__'
 *
 * @return TokenType GLOBAL_ID
 */
Token scan_global_id(void);

//------------------------------------- Numbers --------------------------------------------------

/**
 * @brief Scan exponent notation
 *
 * @return FLOATING token
 */
Token scan_exponent(void);

/**
 * @brief Scans floating notation
 *
 * Can diverge into exponent
 *
 * @return FLOATING token
 */
Token scan_floating(void);

/**
 * @brief Scans hexadecimal notation
 *
 * @return INTEGER token
 */
Token scan_hex(void);

/**
 * @brief Numbers starting with 0
 *
 * Can become floating, hex, or just 0
 *
 * @return INTEGER token
 */
Token scan_zero(void);

//------------------------------------- Strings --------------------------------------------------

/**
 * @brief Process escape sequences in strings
 */
bool handle_escape_sequence(void);

/**
 * @brief Scan normal string
 *
 * @return STRING token or ERROR token if unterminated
 */
Token scan_normal_string(void);

/**
 * @brief Scan multi-line string
 *
 * @return STRING token or ERROR token if unterminated
 */
Token scan_multiline_string(void);

/**
 * @brief Decide if string is normal or multiline
 *
 * @return STRING token
 */
Token scan_string(void);

//------------------------------------- Operators --------------------------------------------------

/**
 * @brief Decide operator characters
 *
 * @param op Character to match
 * @return Operator token
 */
Token scan_operator(char op);

//------------------------------------- Get token --------------------------------------------------

/**
 * @brief Scan next token from input
 *
 * @return Next token
 */
Token get_token(void);

//================================================================================================
//                                      DEBUG
//================================================================================================

void print_token(Token token);
bool parser_function(bool debug);

#endif // SCANNER_H
