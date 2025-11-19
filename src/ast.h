/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) definitions
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the definitions and declarations for the Abstract Syntax Tree (AST)
 */

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ==== AST Node Types =====================
typedef enum {
    NODE_PROGRAM, // Root node of the program
    NODE_FUNCTION, // Function node (static foo(a,b){...})
    NODE_SETTER,   // Setter node
    NODE_GETTER,   // Getter node
    NODE_ID, // Name of variable or function
    NODE_TYPE_ID, // Type identifier (Num, String, Bool, Null)
    NODE_BLOCK, // Block of statements {...}
    NODE_RETURN, // Return statement
    NODE_ASSIGN, // x = expr
    NODE_VAR_DECL, // var x
    NODE_CALL, // Function call foo(a,b)
    NODE_BINOP, // Binary operation (+, -, *, /...)
    NODE_LITERAL, // Literal value (num, string, bool, null)
    NODE_IF, // If statement
    NODE_WHILE, // While loop
    NODE_PARAM_LIST, // Function parameter list
    NODE_ARG_LIST, // Function argument list
    NODE_UNOP,  // Unary operation (e.g., negation)
    NODE_TERNARY  // Ternary operation (condition ? expr1 : expr2)
} ASTNodeType;

// ==== AST Data Types =====================
typedef enum {
    TYPE_UNKNOWN, // Default type (used for untyped nodes)
    TYPE_NULL, // Null type
    TYPE_BOOL, // Result of relational operations
    TYPE_INT, // Integer type
    TYPE_FLOAT, // Floating point type
    TYPE_STRING // String type
} DataType;

// AST Frame Types
typedef enum {
    FRAME_GLOBAL, // Global frame
    FRAME_LOCAL,   // Local frame
    FRAME_UNKNOWN  // Unknown frame
} ASTFrameType;

// ==== AST Node Structure =====================
typedef struct ASTNode {
    ASTNodeType type; // Type of the AST node
    ASTFrameType frame; // Frame type of the AST node
    char *value; // Value of the node (for literals, identifiers...)
    DataType data_type; // Data type of the node
    struct ASTNode **children; // Array of pointers to child nodes
    size_t child_count; // Number of child nodes
    struct ASTNode *parent; // Pointer to parent node
} ASTNode;

// ==== Function Prototypes =====================

/*
 * @brief Create a new AST node
 * @param type Type of the AST node
 * @param value Value of the node
 * @return Pointer to the newly created AST node or NULL on failure
 */
ASTNode *ast_create_node(ASTNodeType type, const char *value, DataType data_type);

/*
 * @brief Add a child node to a parent AST node
 * @param parent Pointer to the parent AST node
 * @param child Pointer to the child AST node
 */
void ast_add_child(ASTNode *parent, ASTNode *child);

/*
 * @brief Free the AST and all its nodes
 * @param root Pointer to the root AST node
 */
void ast_free(ASTNode *root);

/*
 * @brief Debug function to print the AST (for testing purposes)
 * @param node Pointer to the root AST node
 * @param out FILE pointer to print the AST to
 */
void ast_fprint_debug(const ASTNode *node, FILE *out);

// ==== Helper Functions =====================
/*
 * @brief Convert ASTNodeType to string for debugging
 * @param type ASTNodeType to convert
 * @return String representation of the ASTNodeType
 */
const char *ast_node_type_to_string(ASTNodeType type);

/*
 * @brief Convert DataType to string for debugging
 * @param data_type DataType to convert
 * @return String representation of the DataType
 */
const char *data_type_to_string(DataType data_type);

/* @brief Internal function to print the AST recursively
 *
 * @param node Current AST node
 * @param out FILE pointer to print to
 * @param prefix String prefix for formatting
 * @param is_last Boolean indicating if this is the last child
 */
void ast_fprint_debug_inner(const ASTNode *node, FILE *out,
                                   const char *prefix, bool is_last);

/* @brief Print the AST for debugging to stdout
 * @param root Pointer to the root AST node
 */
void ast_print_debug_stdout(const ASTNode *root);

/* @brief Print the AST for debugging to a file
 * @param root Pointer to the root AST node
 * @param filename Name of the file to print to
 * @return 0 on success, -1 on error
 */

#endif // AST_H
