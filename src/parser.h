/**
 * @file parser.h
 * @author Šimon Čorej xcorejs00
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

/*============================== Scanner function declaration ==============================*/
Token get_token();
void scanner_init(FILE* source, FILE* output);

/*============================== Token ===============================*/
Token token;

/*============================== Parser function declaration ===============================*/
ASTNode* cond_loop(int *error_code);
ASTNode* command(int* error_code);
ASTNode* assign(int *error_code);
ASTNode* block(int* error_code);
ASTNode* built_in_call(int* error_code);
ASTNode* func_decl(int* error_code);
ASTNode* func_call(int* error_code);
ASTNode* params(int* error_code);
int valid();
ASTNode* program(int* error_code);
ASTNode* expression(int* error_code);
bool match_token(TokenType type);

/*=================================== Stack Functions for tokens ======================================*/
typedef struct {
    Token* array;
    int topIndex;
} Stack;

int stack_init(Stack* stack);
bool stack_is_empty(const Stack* stack);
int stack_pop(Stack* stack, Token* token_ptr);
int stack_top(Stack* stack, Token* token_ptr);
int stack_push(Stack* stack, Token token);
void stack_destroy(Stack* stack);
// Pomocná fce: najde nejvrchnější terminál (přeskočí < a E)
int stack_top_terminal(Stack* stack, Token* token_ptr);
// Pomocná fce: vloží token PŘED nejvrchnější terminál
int stack_push_before_top_terminal(Stack* stack, Token token);

/*============================== Stack for AST nodes ===============================*/
typedef struct {
    ASTNode** array;
    int topIndex;
} ASTStack;

int ast_stack_init(ASTStack* stack);
bool ast_stack_is_empty(const ASTStack* stack);
int ast_stack_pop(ASTStack* stack, ASTNode** node);
ASTNode* ast_stack_top(ASTStack* stack);
int ast_stack_push(ASTStack* stack, ASTNode* node);
void ast_stack_destroy(ASTStack* stack);

/*=================================== Precedence Table =====================================*/


// Indexy pro tabulku
typedef enum {
    // 0: * /
    IDX_MUL,
    // 1: + -
    IDX_ADD,
    // 2: < > <= >=
    IDX_CMP,
    // 3: is
    IDX_IS,
    // 4: == !=
    IDX_EQ,
    // 5: (
    IDX_LBR,
    // 6: )
    IDX_RBR,
    // 7: ?
    IDX_QMARK,
    // 8: :
    IDX_COLON,
    // 9: id, literal, Ifj.call() ...
    IDX_OPERAND,
    // 10: $ (konec vyrazu)
    IDX_END,
    IDX_COUNT // Pocet sloupcu/radku
} PrecTableIndex;

// // Speciální tokeny pro precedenční analýzu
// #define PREC_ERR  (TokenType)100 // 'X' - Chyba
// #define PREC_SHIFT (TokenType)101 // '<' - Shift
// #define PREC_REDUCE (TokenType)102 // '>' - Reduce
// #define PREC_EQUAL (TokenType)103 // '=' - Handle (pro zavorky)
// #define PREC_PUSH (TokenType)104 // 'P' - Special (pro ternarni operator)

// Zkraceni pro lepsi citelnost
#define X PREC_ERR
#define S PREC_SHIFT
#define R PREC_REDUCE
#define E PREC_EQUAL
#define P PREC_PUSH

// [NA ZÁSOBNÍKU] [NA VSTUPU]
//             * /   + -   < >   is    == !=   (     )     ?     :     id    $
TokenType precedence_table[IDX_COUNT][IDX_COUNT] = {
    /* * / */ {R, R, R, R, R, S, R, R, R, S, R},
    /* + - */ {S, R, R, R, R, S, R, R, R, S, R},
    /* < > */ {S, S, R, R, R, S, R, R, R, S, R},
    /* is  */ {S, S, S, R, R, S, R, R, R, S, R},
    /* == !=*/{S, S, S, S, R, S, R, R, R, S, R},
    /* (   */ {S, S, S, S, S, S, X, S, E, S, E}, // E u ':' a ')'
    /* )   */ {R, R, R, R, R, X, R, R, R, X, R},
    /* ?   */ {S, S, S, S, S, S, S, S, P, S, X}, // P u ':'
    /* :   */ {R, R, R, R, R, X, R, R, R, X, R},
    /* id  */ {R, R, R, R, R, X, R, R, R, X, R},
    /* $   */ {S, S, S, S, S, S, X, S, X, S, X}  // X na ')' a '$'
};

// Funkce pro mapovani tokenu na index tabulky
int token_to_int(Token in_token);

#endif // PARSER_H
