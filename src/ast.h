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
    NODE_BLOCK, // Block of statements {...}
    NODE_RETURN, // Return statement
    NODE_ASSIGN, // x = expr
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
} ASTDataType;

// ==== AST Node Structure =====================
typedef struct ASTNode {
    ASTNodeType type; // Type of the AST node
    char *value; // Value of the node (for literals, identifiers...)
    ASTDataType data_type; // Data type of the node
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
ASTNode *ast_create_node(ASTNodeType type, const char *value);

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
 * @param depth Current depth in the AST (for indentation)
 */
void ast_print_debug(const ASTNode *node, int depth);


#endif // AST_H
