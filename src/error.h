/**
 * @file error.h
 * @brief Error handling definitions
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the definitions and declarations for error handling in the IFJ project.
 */

#ifndef IFJ_ERROR_H
#define IFJ_ERROR_H

typedef enum {
  ERR_OK = 0,                    // No error
  LEXICAL_ERROR = 1,             // Lexical error (invalid token)
  SYNTAX_ERROR = 2,              // Syntax error (parsing error)
  ERR_SEMANTIC_UNDEFINED = 3,    // Semantic error: Undefined identifier or function
  ERR_SEMANTIC_REDEFINITION = 4, // Semantic error: Redefinition of identifier or function
  ERR_SEMANTIC_FUNCTION = 5, // Semantic error: Invalid number of function arguments or invalid type in built-in function call
  ERR_SEMANTIC_TYPE = 6,   // Semantic error: Type mismatch
  ERR_SEMANTIC_OTHER = 10, // Semantic error: Other semantic error
  ERR_INTERNAL = 99        // Internal error (e.g., memory allocation failure)
} ErrorCode;

#endif // IFJ_ERROR_H
