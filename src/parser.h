/**
 * @file parser.h
 * @author Šimon Čorej xcorejs00
 * @author Patrik Lošťák (xlostap00)
 * @brief Parser header file
 * @date 2025-10-28
 *
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "scanner.h"
#include "error.h"
#include "ast.h"
#include "strutils.h"

#define STACK_SIZE 100

/*============================== Token ===============================*/
extern Token token;

/*============================== Parser functions declaration ===============================*/

/*
* @ brief Runs the parser on the given input file and produces an AST.
* @ param input_file Pointer to the input file to parse.
* @ param output_file Pointer to the output file for any parser output.
* @ param parse_error_code Pointer to an integer to store error codes.
* @ return Pointer to the root ASTNode of the parsed program or NULL on error.
*/
ASTNode* run_parser(FILE* input_file, FILE* output_file, int* parse_error_code);

/*
* @ brief Parses conditional and loop statements.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the conditional or loop statement.
*/
ASTNode* cond_loop(int *error_code);

/*
* @ brief Parses a single command in the source code.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the command.
*/
ASTNode* command(int* error_code);

/*
* @ brief Parses an assignment statement.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the assignment.
*/
ASTNode* assign(int *error_code);

/*
* @ brief Parses a block of code.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the block.
*/
ASTNode* block(int* error_code);

/*
* @ brief Parses built-in function calls.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the built-in function call.
*/
ASTNode* built_in_call(int* error_code);

/*
* @ brief Parses a function declaration.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the function declaration.
*/
ASTNode* func_decl(int* error_code);

/*
* @ brief Parses a function call.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the function call.
*/
ASTNode* func_call(int* error_code);

ASTNode* params(int* error_code);


/*
* @ brief Validates the prolog of the source code.
* @ param None.
* @ return Integer indicating validity (e.g., 1 for valid, 0 for invalid).
*/
int valid();

/*
* @ brief Parses the entire program.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the entire program.
*/
ASTNode* program(int* error_code);

/*
* @ brief Parses an expression using precedence parsing.
* @ param error_code Pointer to an integer to store error codes.
* @ return Pointer to the ASTNode representing the expression.
*/
ASTNode* parse_expression(int* error_code);


/*
* @ brief Checks if the current token matches the expected type.
* @ param type The expected token type.
* @ return Boolean indicating whether the token matches.
*/
bool match_token(TokenType type);

/*=================================== Stack Functions for tokens ======================================*/
typedef struct {
    Token* array;
    int topIndex;
} Stack;


/*
* @ brief Checks if the stack is empty.
* @ param stack Pointer to the stack to check.
* @ return Boolean indicating whether the stack is empty.
*/
bool stack_is_empty(const Stack* stack);

/*
* @ brief Pops the top token from the stack.
* @ param stack Pointer to the stack.
* @ param token_ptr Pointer to store the popped token.
* @ return Integer indicating success or failure.
*/
int stack_pop(Stack* stack, Token* token_ptr);

/*
* @ brief Retrieves the top token from the stack without removing it.
* @ param stack Pointer to the stack.
* @ param token_ptr Pointer to store the top token.
* @ return Integer indicating success or failure.
*/
int stack_top(Stack* stack, Token* token_ptr);

/*
* @ brief Pushes a token onto the stack.
* @ param stack Pointer to the stack.
* @ param token The token to push.
* @ return Integer indicating success or failure.
*/
int stack_push(Stack* stack, Token token);

/*
* @ brief Destroys the stack and frees resources.
* @ param stack Pointer to the stack to destroy.
*/
void stack_destroy(Stack* stack);

/*
* @ brief Finds the topmost terminal token on the stack (skipping < and E).
* @ param stack Pointer to the stack.
* @ param token_ptr Pointer to store the found terminal token.
* @ return Integer indicating success or failure.
*/
int stack_top_terminal(Stack* stack, Token* token_ptr);

/*============================== Stack for AST nodes ===============================*/
typedef struct {
    ASTNode** array;
    int topIndex;
} ASTStack;

/*
* @ brief Initializes the AST node stack.
* @ param stack Pointer to the ASTStack to initialize.
* @ return Integer indicating success or failure.
*/
int ast_stack_init(ASTStack* stack);

/*
* @ brief Checks if the AST node stack is empty.
* @ param stack Pointer to the ASTStack to check.
* @ return Boolean indicating whether the stack is empty.
*/
bool ast_stack_is_empty(const ASTStack* stack);

/*
* @ brief Pops the top AST node from the stack.
* @ param stack Pointer to the ASTStack.
* @ param node Pointer to store the popped ASTNode.
* @ return Integer indicating success or failure.
*/
int ast_stack_pop(ASTStack* stack, ASTNode** node);

/*
* @ brief Retrieves the top AST node from the stack without removing it.
* @ param stack Pointer to the ASTStack.
* @ return Pointer to the top ASTNode, or NULL if the stack is empty.
*/
ASTNode* ast_stack_top(ASTStack* stack);

/*
* @ brief Pushes an AST node onto the stack.
* @ param stack Pointer to the ASTStack.
* @ param node The ASTNode to push.
* @ return Integer indicating success or failure.
*/
int ast_stack_push(ASTStack* stack, ASTNode* node);

/*
* @ brief Destroys the AST node stack and frees resources.
* @ param stack Pointer to the ASTStack to destroy.
*/
void ast_stack_destroy(ASTStack* stack);

/*=================================== Precedence Table =====================================*/


// Indexy pro tabulku
typedef enum {
    // Unary (nejvyssi)
    IDX_NOT,     // 0: ! (a unarni -)
    // Binarni (serazeno dle priority)
    IDX_MUL,     // 1: * /
    IDX_ADD,     // 2: + -
    IDX_CMP,     // 3: < > <= >=
    IDX_IS,      // 4: is
    IDX_EQ,      // 5: == !=
    IDX_AND,     // 6: &&
    IDX_OR,      // 7: ||
    // Ostatni symboly
    IDX_LBR,     // 8: (
    IDX_RBR,     // 9: )
    IDX_QMARK,   // 10: ?
    IDX_COLON,   // 11: :
    IDX_OPERAND, // 12: id, literal, ...
    IDX_TYPE,    // 13: type (Num, String, Bool, Null)
    IDX_END,     // 14: $ (konec vyrazu)
    IDX_COUNT    // 15: Pocet indexu
} PrecTableIndex;


// Symbols for precedence parsing
#define X PREC_ERR // Error
#define S PREC_SHIFT // Shift ">"
#define R PREC_REDUCE // Reduce "<"
#define E PREC_EQUAL // Equal "="

// Precedence table definition
extern TokenType precedence_table[IDX_COUNT][IDX_COUNT];

/*
* @ brief Maps a token to its corresponding index in the precedence table.
* @ param in_token The token to map.
* @ return The index in the precedence table, or -1 if unknown.
*/
int token_to_int(Token in_token);

#endif // PARSER_H
