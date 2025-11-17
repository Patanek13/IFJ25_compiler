/**
 * @file parser.c
 * @author Šimon Čorej (xcorejs00)
 * @author Patrik Lošťák (xlostap00)
 * @brief Main Parser Function
 * @version 0.1
 * @date 2025-10-12
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"

//TODO: odstranit debug vypisy, jelikoz se vypisuji do stderr v main.c
//TODO: otestovat vsechny mozne vyrazy a operatory
//TODO: nefunguje unarni minus skrz scanner (scan_minus)

FILE* in;
FILE* out;

Token token;

// Precedence table for operators
// [NA ZÁSOBNÍKU] [NA VSTUPU]
//             !     * /   + -   < >   is    == !=  &&    ||    (     )     ?     :     id    $
TokenType precedence_table[IDX_COUNT][IDX_COUNT] = {
    /* 0 !   */ {S, S, S, S, S, S, S, S, S, R, R, R, S, R}, // Unary ma vyssi prioritu nez vse
    /* 1 * / */ {R, R, R, R, R, R, R, R, S, R, R, R, S, R},
    /* 2 + - */ {S, S, R, R, R, R, R, R, S, R, R, R, S, R},
    /* 3 < > */ {S, S, S, R, R, R, R, R, S, R, R, R, S, R},
    /* 4 is  */ {S, S, S, S, R, R, R, R, S, R, R, R, S, R},
    /* 5 == !=*/{S, S, S, S, S, R, R, R, S, R, R, R, S, R},
    /* 6 &&  */ {S, S, S, S, S, S, R, R, S, R, R, R, S, R},
    /* 7 ||  */ {S, S, S, S, S, S, S, R, S, R, R, R, S, R},
    /* 8 (   */ {S, S, S, S, S, S, S, S, S, E, S, E, S, X}, // ( vs ) -> E, ( vs : -> E
    /* 9 )   */ {R, R, R, R, R, R, R, R, X, R, R, R, X, R},
    /* 10?   */ {S, S, S, S, S, S, S, S, S, S, S, P, S, X}, // ? vs : -> P
    /* 11:   */ {R, R, R, R, R, R, R, R, X, R, X, R, S, R}, // : redukuje jen po E, shiftuje vsechno ostatni
    /* 12id  */ {R, R, R, R, R, R, R, R, X, R, R, R, X, R},
    /* 13$   */ {S, S, S, S, S, S, S, S, S, X, S, X, S, X}  // $ (dno) shiftuje vse
};

/* condition state 0 = IF 1 = WHILE */
bool cond_state = 0;
char* built_in_string[] = {"read_str", "read_num", "write", "floor", "str", "length", "substring", "strcmp", "ord", "chr", "read_bool"};

bool match_token(TokenType type){
    token = get_token();
    return (token.type == type);
}

// debug token
bool check_and_take_token(TokenType type, int *error_code){
    // Check for lexical errors
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return false;
    }

    if (token.type != type){
        fprintf(out, "ERROR: Ocekavam token type %d ale nasel jsem %d\n", type, token.type);
        *error_code = SYNTAX_ERROR;
        return false;
    }

    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return false;
    }

    return true;
}

bool is_built_in_func(){
    for (int i = 0; i < 11; i++){
        if (strcmp(token.value.string, built_in_string[i]) == 0){ return true; }
    }
    return false;
}


// Pomocna funkce pro debug vypisy
static const char* token_type_to_string(TokenType type) {
    switch(type) {
        case BLOCK_START: return "BLOCK_START";
        case BLOCK_END: return "BLOCK_END";
        case BRACKET_START: return "BRACKET_START";
        case BRACKET_END: return "BRACKET_END";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULTIPLY: return "MULTIPLY";
        case DIVIDE: return "DIVIDE";
        case DOT: return "DOT";
        case COMMA: return "COMMA";
        case COLON: return "COLON";
        case QUESTION: return "QUESTION";
        case NEW_LINE: return "NEW_LINE";
        case EQUAL: return "EQUAL";
        case EQUAL_EQUAL: return "EQUAL_EQUAL";
        case LESS: return "LESS";
        case LESS_EQUAL: return "LESS_EQUAL";
        case MORE: return "MORE";
        case MORE_EQUAL: return "MORE_EQUAL";
        case NOT: return "NOT";
        case NOT_EQUAL: return "NOT_EQUAL";
        case AND: return "AND";
        case OR: return "OR";
        case ID: return "ID";
        case GLOBAL_ID: return "GLOBAL_ID";
        case INTEGER: return "INTEGER";
        case FLOATING: return "FLOATING";
        case STRING: return "STRING";
        case BOOLEAN: return "BOOLEAN";
        case OPERATOR: return "OPERATOR";
        case PSEUDO_E: return "E (PSEUDO)";
        case MARKER_LESS: return "< (MARKER)";
        case CLASS: return "CLASS";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case IS: return "IS";
        case NULL_KEYWORD: return "NULL_KEYWORD";
        case RETURN: return "RETURN";
        case VAR: return "VAR";
        case WHILE: return "WHILE";
        case IFJ: return "IFJ";
        case STATIC: return "STATIC";
        case IMPORT: return "IMPORT";
        case FOR: return "FOR";
        case NUM_TYPE: return "NUM_TYPE";
        case STR_TYPE: return "STR_TYPE";
        case NULL_TYPE: return "NULL_TYPE";
        case BOOL_TYPE: return "BOOL_TYPE";
        case END_EXPR: return "END_EXPR ($)";
        case EOF_TOKEN: return "EOF_TOKEN";
        case ERROR: return "ERROR";
        // tokeny pro precedencni analyzu
        case PREC_ERR: return "PREC_ERR";
        case PREC_SHIFT: return "PREC_SHIFT";
        case PREC_REDUCE: return "PREC_REDUCE";
        case PREC_EQUAL: return "PREC_EQUAL";
        case PREC_PUSH: return "PREC_PUSH";
        default: return "UNKNOWN_TOKEN";
    }
}

int ast_stack_init(ASTStack* stack) {
    if (stack == NULL) return SYNTAX_ERROR;
    stack->array = (ASTNode**)malloc(STACK_SIZE * sizeof(ASTNode*));
    if (stack->array == NULL) return SYNTAX_ERROR;
    stack->topIndex = -1;
    return ERR_OK;
}

bool ast_stack_is_empty(const ASTStack* stack) {
    return stack->topIndex == -1;
}

int ast_stack_pop(ASTStack* stack, ASTNode** node) {
    if (!ast_stack_is_empty(stack)) {
        *node = stack->array[stack->topIndex--];
        return ERR_OK;
    }
    return SYNTAX_ERROR;
}

ASTNode* ast_stack_top(ASTStack* stack) {
    if (!ast_stack_is_empty(stack)) {
        return stack->array[stack->topIndex];
    }
    return NULL;
}

int ast_stack_push(ASTStack* stack, ASTNode* node) {
    if (stack->topIndex >= STACK_SIZE - 1) return SYNTAX_ERROR;
    stack->array[++stack->topIndex] = node;
    return ERR_OK;
}

void ast_stack_destroy(ASTStack* stack) {
    // Pozor: Tato funkce nemaze samotne AST uzly,
    // ty jsou soucasti stromu, ktery mazeme az na konci.
    free(stack->array);
    stack->array = NULL;
    stack->topIndex = -1;
}

// ===================================================================
// Stack implementation for tokens
// ===================================================================

int stack_init(Stack* stack) {
    if (stack == NULL) return SYNTAX_ERROR;
    stack->array = (Token*)malloc(STACK_SIZE * sizeof(Token));
    if (stack->array == NULL) return SYNTAX_ERROR;
    stack->topIndex = -1;
    return ERR_OK;
}

bool stack_is_empty(const Stack* stack) {
    return stack->topIndex == -1;
}

int stack_pop(Stack* stack, Token* token_ptr) {
    if (!stack_is_empty(stack)) {
        if (token_ptr) *token_ptr = stack->array[stack->topIndex];
        stack->topIndex--;
        return ERR_OK;
    }
    return SYNTAX_ERROR;
}

int stack_top(Stack* stack, Token* token_ptr) {
    if (!stack_is_empty(stack)) {
        *token_ptr = stack->array[stack->topIndex];
        return ERR_OK;
    }
    return SYNTAX_ERROR;
}

int stack_push(Stack* stack, Token token) {
    if (stack->topIndex >= STACK_SIZE - 1) return SYNTAX_ERROR;
    stack->array[++stack->topIndex] = token;
    return ERR_OK;
}

void stack_destroy(Stack* stack) {
    free(stack->array);
    stack->array = NULL;
    stack->topIndex = -1;
}

// Najde nejvrchnejsi *terminal* na zasobniku (preskoci znacky)
int stack_top_terminal(Stack* stack, Token* token_ptr) {
    if (stack_is_empty(stack)) return SYNTAX_ERROR;
    for (int i = stack->topIndex; i >= 0; i--) {
        TokenType type = stack->array[i].type;
        // Preskocime pseudo neterminal
        if (type == PSEUDO_E || type == MARKER_LESS) {
            continue;
        }
        // Nasli jsme prvni terminal
        *token_ptr = stack->array[i];
        return ERR_OK;
    }
    return SYNTAX_ERROR; // Nenasel se zadny terminal
}

// Pomocna funkce pro debug vypis zasobniku tokenu
void print_token_stack(Stack* stack) {
    fprintf(out, "--- STACK TOP ---\n");
    if (stack_is_empty(stack)) {
        fprintf(out, "(prazdny)\n");
    }
    for (int i = stack->topIndex; i >= 0; i--) {
        fprintf(out, "  %d: %s\n", i, token_type_to_string(stack->array[i].type));
    }
    fprintf(out, "--- STACK BOTTOM ---\n");
}


// ===================================================================
// Implementace nove token_to_int
// ===================================================================

int token_to_int(Token in_token) {
    fprintf(out, ">> token_to_int: Zpracovávám token %s\n", token_type_to_string(in_token.type));
    switch (in_token.type) {
        case ERROR:
            return -2; // Error: invalid token

        case NOT:
            return IDX_NOT; // 0

        case OPERATOR:
            // Unarni minus resime specialne v 'parse_expression'
            if (strcmp(in_token.value.string, "*") == 0 || strcmp(in_token.value.string, "/") == 0)
                return IDX_MUL; // 1
            if (strcmp(in_token.value.string, "+") == 0 || strcmp(in_token.value.string, "-") == 0)
                return IDX_ADD; // 2
            break;

        case LESS:
        case MORE:
        case LESS_EQUAL:
        case MORE_EQUAL:
            return IDX_CMP; // 3

        case IS:
            return IDX_IS; // 4

        case EQUAL_EQUAL:
        case NOT_EQUAL:
            return IDX_EQ; // 5

        case AND:
            return IDX_AND; // 6

        case OR:
            return IDX_OR; // 7

        case BRACKET_START:
            return IDX_LBR; // 8

        case BRACKET_END:
            return IDX_RBR; // 9

        case QUESTION:
            return IDX_QMARK; // 10

        case COLON:
            return IDX_COLON; // 11

        case ID:
        case GLOBAL_ID:
        case STRING:
        case INTEGER:
        case FLOATING:
        case BOOLEAN:
        case NULL_KEYWORD:
        case IFJ:
        case NUM_TYPE:
        case STR_TYPE:
        case NULL_TYPE:
        case BOOL_TYPE:
            return IDX_OPERAND; // 12

        // konec vyrazu
        case NEW_LINE:
        case BLOCK_START:
        case COMMA:
        case EOF_TOKEN:
        case END_EXPR:
            return IDX_END; // 13

        default:
            return -1;
    }
    return -1;
}

// ===================================================================
// Helpers for expression parsing
// ===================================================================

// Pretvori jednoduchy token na AST uzel (pro operandy)
// Logiku pro ID/IFJ se resim v hlavnim loopu parse_expression
static ASTNode* create_ast_node_from_token(Token t, int* error_code) {
    ASTNode* node = NULL;
    *error_code = ERR_OK;

    // Check error token
    if (t.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    switch (t.type) {
        // ID je ted reseno v hlavni smycce, protoze musime
        // rozlisit 'x' od 'x('
        case GLOBAL_ID:
            node = ast_create_node(NODE_ID, t.value.string, TYPE_UNKNOWN);
            break;

        case STRING:
            node = ast_create_node(NODE_LITERAL, t.value.string, TYPE_STRING);
            break;

        case INTEGER: {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "%d", t.value.integer);
            node = ast_create_node(NODE_LITERAL, buffer, TYPE_INT);
            break;
        }

        case FLOATING: {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "%f", t.value.floating);
            node = ast_create_node(NODE_LITERAL, buffer, TYPE_FLOAT);
            break;
        }

        case NULL_KEYWORD:
            node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
            break;
        case BOOLEAN: {
            char* bool_str = t.value.boolean ? "true" : "false";
            node = ast_create_node(NODE_LITERAL, bool_str, TYPE_BOOL);
            break;
        }
        case NUM_TYPE:
            node = ast_create_node(NODE_ID, "Num", TYPE_UNKNOWN);
            break;
        case STR_TYPE:
            node = ast_create_node(NODE_ID, "String", TYPE_UNKNOWN);
            break;
        case NULL_TYPE:
            node = ast_create_node(NODE_ID, "Null", TYPE_UNKNOWN);
            break;
        case BOOL_TYPE:
            node = ast_create_node(NODE_ID, "Bool", TYPE_UNKNOWN);
            break;

        default:
            fprintf(out, "ERROR: Neocekavany typ v create_ast_node_from_token: %d\n", t.type);
            *error_code = SYNTAX_ERROR;
            return NULL;
    }

    if (node == NULL) {
        *error_code = ERR_INTERNAL;
    }

    // Musime spotrebovat token, ktery jsme zpracovali
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        ast_free(node);
        return NULL;
    }
    return node;
}


// Hlavni funkce pro redukci
static int do_reduction(Stack* tokenStack, ASTStack* astStack, int* error_code) {
    fprintf(out, "DEBUG: Redukuji...\n");
    //print_token_stack(tokenStack); // Debug vypis

    Token handle[10];
    int handle_len = 0;
    Token t;

    // 1. Popni handle (vse nad znackou '<')
    while(true) {
        if(stack_pop(tokenStack, &t) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }
        if (t.type == MARKER_LESS) { break; } // Nasli jsme zacatek handle
        handle[handle_len++] = t;
        if (handle_len > 9) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }
    }

    ASTNode* new_node = NULL;
    // Handle je nacteny v opacnem poradi (vrchol zasobniku je na handle[0])

    // Pravidlo 1: E -> operand (i, literal, atd.)
    if (handle_len == 1 && token_to_int(handle[0]) == IDX_OPERAND) {
        // new_node = NULL; // Uzel uz je na astStack, nic nedelame
        // Debug vypis
        //fprintf(out, "DEBUG do_reduction: Rozpoznano pravidlo E -> operand (len 1)\n");
    }

    // Pravidlo 2: E -> ( E )
    else if (handle_len == 3 && handle[2].type == BRACKET_START && handle[1].type == PSEUDO_E && handle[0].type == BRACKET_END) {
        // new_node = NULL; // Uzel uz je na astStack, nic nedelame
        // Debug vypis
        //fprintf(out, "DEBUG do_reduction: Rozpoznano pravidlo E -> ( E ) (len 3)\n");
    }
    // Pravidlo 3 a 4: E -> op E (unarni)  NEBO E -> E op E (binarni)
    // Handle (reversed) je [E, op]
    else if (handle_len == 2 && handle[0].type == PSEUDO_E && token_to_int(handle[1]) != IDX_OPERAND) {
        Token op_token = handle[1];
        bool is_unary = false;

        if (op_token.type == NOT) {
            is_unary = true;
        } else if (op_token.type == OPERATOR && strcmp(op_token.value.string, "-") == 0) {
            Token next_on_stack;
            // podivame se na dalsi token na zasobniku
            if (stack_top(tokenStack, &next_on_stack) == ERR_OK && next_on_stack.type == PSEUDO_E) {
                // Na zasobniku je dalsi E. Je to BINARNI E - E.
                is_unary = false;
            } else {
                // Cokoliv jineho (dalsi operator, $, '('...) znamena UNARNI minus.
                is_unary = true;
            }
        } else {
            // Vsechny ostatni operatory (is, ==, &&, +, *) jsou vzdy BINARNI
            is_unary = false;
        }

        // Je to unarni
        if (is_unary) {
            fprintf(out, "DEBUG do_reduction: Rozpoznano pravidlo UNARY (len 2)\n");
            ASTNode *operand;
            if (ast_stack_pop(astStack, &operand) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }

            char* op_str = (op_token.type == NOT) ? "!" : "-";
            new_node = ast_create_node(NODE_UNOP, op_str, TYPE_UNKNOWN);
            if (new_node == NULL) { *error_code = ERR_INTERNAL; ast_free(operand); return ERR_INTERNAL; }
            ast_add_child(new_node, operand);

        } else { // Je to binarni
            fprintf(out, "DEBUG do_reduction: Rozpoznano pravidlo BINARY (len 2)\n");
            Token E_left_token;
            if (stack_pop(tokenStack, &E_left_token) != ERR_OK || E_left_token.type != PSEUDO_E) {
                fprintf(out, "ERROR: Pri redukci E op E (len 2) chybi E_left na zasobniku.\n");
                *error_code = SYNTAX_ERROR; return SYNTAX_ERROR;
            }

            ASTNode *right, *left;
            if (ast_stack_pop(astStack, &right) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }
            if (ast_stack_pop(astStack, &left) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }

            char* op_str = NULL;
            if (op_token.type == OPERATOR) op_str = op_token.value.string;
            else if (op_token.type == IS) op_str = "is";
            else if (op_token.type == EQUAL_EQUAL) op_str = "==";
            else if (op_token.type == NOT_EQUAL) op_str = "!=";
            else if (op_token.type == LESS) op_str = "<";
            else if (op_token.type == MORE) op_str = ">";
            else if (op_token.type == LESS_EQUAL) op_str = "<=";
            else if (op_token.type == MORE_EQUAL) op_str = ">=";
            else if (op_token.type == AND) op_str = "&&";
            else if (op_token.type == OR) op_str = "||";
            else { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }

            new_node = ast_create_node(NODE_BINOP, op_str, TYPE_UNKNOWN);
            if (new_node == NULL) { *error_code = ERR_INTERNAL; ast_free(left); ast_free(right); return ERR_INTERNAL; }
            ast_add_child(new_node, left);
            ast_add_child(new_node, right);
        }
    }


    // Pravidlo 5: E -> E op E (kde E_left a E_right jsou *nad* znackou <)
    // Handle (reversed) je [E_right, op, E_left]
    else if (handle_len == 3 && handle[0].type == PSEUDO_E && handle[2].type == PSEUDO_E) {
        Token op_token = handle[1];

        // Debug vypis
        //fprintf(out, "DEBUG do_reduction: Rozpoznano pravidlo E -> E op E (len 3)\n");

        ASTNode *right, *left;
        if (ast_stack_pop(astStack, &right) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; } // Pop E_right
        if (ast_stack_pop(astStack, &left) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; } // Pop E_left

        char* op_str = NULL;
        if (op_token.type == OPERATOR) op_str = op_token.value.string;
        else if (op_token.type == AND) op_str = "&&";
        else if (op_token.type == OR) op_str = "||";
        else { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; } // jine operatory tu nemaj co delat

        new_node = ast_create_node(NODE_BINOP, op_str, TYPE_UNKNOWN);
        if (new_node == NULL) { *error_code = ERR_INTERNAL; ast_free(left); ast_free(right); return ERR_INTERNAL; }
        ast_add_child(new_node, left);
        ast_add_child(new_node, right);
    }

    // Pravidlo 6: E -> E ? E : E
    else if (handle_len == 4 && handle[3].type == QUESTION && handle[2].type == PSEUDO_E && handle[1].type == COLON && handle[0].type == PSEUDO_E) {

         //E_cond (podminka) je *pod* znackou '<'.
        // Musime ji popnout.
        Token E_cond_token;
        if (stack_pop(tokenStack, &E_cond_token) != ERR_OK || E_cond_token.type != PSEUDO_E) {
          fprintf(out, "ERROR: Pri redukci E ? E : E chybi E_condition na zasobniku.\n");
          *error_code = SYNTAX_ERROR; return SYNTAX_ERROR;
        }

        // popneme E_false, E_true a E_condition z astStacku
        ASTNode *false_expr, *true_expr, *condition;
        if (ast_stack_pop(astStack, &false_expr) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }
        if (ast_stack_pop(astStack, &true_expr) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }
        if (ast_stack_pop(astStack, &condition) != ERR_OK) { *error_code = SYNTAX_ERROR; return SYNTAX_ERROR; }

        new_node = ast_create_node(NODE_TERNARY, NULL, TYPE_UNKNOWN);
        if (new_node == NULL) { *error_code = ERR_INTERNAL; return ERR_INTERNAL; }
        ast_add_child(new_node, condition);
        ast_add_child(new_node, true_expr);
        ast_add_child(new_node, false_expr);
    }

    else {
        fprintf(out, "ERROR: Neznamy handle pri redukci (delka %d)\n", handle_len);
        // Vypis handle pro debug
        for(int i=0; i < handle_len; i++) {
            fprintf(out, "  handle[%d] = %s\n", i, token_type_to_string(handle[i].type));
        }
        *error_code = SYNTAX_ERROR;
        return SYNTAX_ERROR;
    }

    // 3. Vloz novy uzel na astStack (pokud nejaky vznikl)
    if (new_node != NULL) {
        if (ast_stack_push(astStack, new_node) != ERR_OK) {
             *error_code = ERR_INTERNAL; return ERR_INTERNAL;
        }
    }

    // 4. Vloz E (pseudo-token) na tokenStack
    Token pseudo_E;
    pseudo_E.type = PSEUDO_E; // Nas neterminal 'E'
    if (stack_push(tokenStack, pseudo_E) != ERR_OK) {
        *error_code = ERR_INTERNAL; return ERR_INTERNAL;
    }

    // Debug vypis
    //fprintf(out, "DEBUG: Vlozen pseudo-token E na tokenStack.\n");
    //print_token_stack(tokenStack);

    fprintf(out, "DEBUG: Redukce uspesna.\n");
    return ERR_OK;
}


// ===================================================================
// Main expression parser function
// ===================================================================

ASTNode* parse_expression(int *error_code) {
    fprintf(out, "DEBUG: parse_expression (Tabulkova metoda) volani\n");
    *error_code = ERR_OK;

    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    Stack tokenStack;
    ASTStack astStack;
    ASTNode* final_node = NULL;

    if (stack_init(&tokenStack) != ERR_OK) {
        *error_code = ERR_INTERNAL; return NULL;
    }
    if (ast_stack_init(&astStack) != ERR_OK) {
        stack_destroy(&tokenStack);
        *error_code = ERR_INTERNAL; return NULL;
    }

    // 1. Vloz $ na tokenStack
    Token end_marker;
    end_marker.type = END_EXPR; // Pouziju END_EXPR jako $
    if(stack_push(&tokenStack, end_marker) != ERR_OK) {
        *error_code = ERR_INTERNAL;
        goto cleanup;
    }

    bool success = false;
    bool is_new_token = true;

    do {
        // 'a' = nejvrchnejsi terminal na zasobniku
        Token top_terminal;
        if(stack_top_terminal(&tokenStack, &top_terminal) != ERR_OK) {
            *error_code = SYNTAX_ERROR;
            goto cleanup;
        }

        // 'b' = aktualni token na vstupu
        Token current_token = token;

        int top_idx = token_to_int(top_terminal);
        int current_idx;

        if (is_new_token) {
        // Specialni osetreni pro minus (unarni vs binarni)
        if (current_token.type == OPERATOR && strcmp(current_token.value.string, "-") == 0) {
            // Je to minus. Je unarni nebo binarni?
            if (top_idx == IDX_OPERAND || top_idx == IDX_RBR) {
                // Predchozi token byl operand nebo ')', takze je to BINARNI minus
                current_idx = IDX_ADD; // Priorita 2
            } else {
                // Predchozi token byl '(', '$' nebo jiny operator, je to UNARNI minus
                current_idx = IDX_NOT; // Priradime mu prioritu 0 (stejne jako !)
            }
        } else {
            // Neni to minus, mapuj normalne
            current_idx = token_to_int(current_token);
        }
      }

        // Zkontroluj konec pred kouknutim do tabulky
        if (top_terminal.type == END_EXPR && (current_idx == IDX_END || current_idx == IDX_RBR)) {
            success = true;
            break; // HOTOVO! (Prijimame $ na zasobniku a $ nebo ')' na vstupu)
        }

        fprintf(out, "DEBUG: Top terminal: %s, Current token: %s\n", token_type_to_string(top_terminal.type), token_type_to_string(current_token.type));

        // Handle lex errors from token_to_int
        if (current_idx == -2) {
            *error_code = LEXICAL_ERROR;
            goto cleanup;
        }

        if (top_idx == -1 || current_idx == -1) { // Syntax error
             fprintf(out, "ERROR: Neznamy token v precedencni analyze. Top: %s, Curr: %s\n", token_type_to_string(top_terminal.type), token_type_to_string(current_token.type));
             *error_code = SYNTAX_ERROR;
             goto cleanup;
        }

        TokenType rule = precedence_table[top_idx][current_idx];
        fprintf(out, "\n!!! DEBUG: ČTENÍ PRECEDENČNÍ TABULKY !!!\n");
        fprintf(out, "    -> Top index (zásobník): %d (Token: %s)\n", top_idx, token_type_to_string(top_terminal.type));
        fprintf(out, "    -> Curr index (vstup):   %d (Token: %s)\n", current_idx, token_type_to_string(current_token.type));
        fprintf(out, "    -> Pravidlo: %s\n", (rule == S) ? "SHIFT <" :
                                         (rule == P) ? "PUSH P" :
                                         (rule == E) ? "EQUAL =" :
                                         (rule == R) ? "REDUCE >" : "ERROR X");
        switch (rule) {
            case S: // Shift '<'
                fprintf(out, "DEBUG: Pravidlo SHIFT <\n");
                {
                    Token less_marker;
                    less_marker.type = MARKER_LESS; // 'LESS'
                    if(stack_push(&tokenStack, less_marker) != ERR_OK) {
                        *error_code = ERR_INTERNAL;
                        goto cleanup;
                    }
                }
                // FALLTHROUGH - pokracuj jako PUSH

            case P: // Push (pro ternarni operator)
            case E: // Equal '=' (pro zavorky)
                is_new_token = true;
                if(rule == E) fprintf(out, "DEBUG: Pravidlo EQUAL =\n");
                else if(rule == P) fprintf(out, "DEBUG: Pravidlo PUSH P\n");

                // Push 'b' (current_token) na tokenStack
                if(stack_push(&tokenStack, current_token) != ERR_OK) {
                    *error_code = ERR_INTERNAL;
                    goto cleanup;
                }

                // --- Zpracovani AST ---
                if (current_idx == IDX_OPERAND) {
                    ASTNode* node = NULL;

                    if (current_token.type == ID) {
                        // Mame ID, musime zjistit, jestli je to volani funkce nebo promenna
                        Token next_token = get_token(); // Spotrebujeme ID, nacteme dalsi

                        // Check for lexical errors after consuming token
                        if (next_token.type == ERROR) {
                            *error_code = LEXICAL_ERROR;
                            goto cleanup;
                        }

                        if (next_token.type == BRACKET_START) {
                            // Je to volani funkce!
                            fprintf(out, "DEBUG: Zpracovavam func_call (inline) uvnitr vyrazu\n");

                            // Vytvorime uzel CALL
                            node = ast_create_node(NODE_CALL, NULL, TYPE_UNKNOWN);
                            if(!node) { *error_code = ERR_INTERNAL;
                              goto cleanup;
                            }

                            // 1. Pridame ID (jmeno funkce)
                            ASTNode* id_node = ast_create_node(NODE_ID, current_token.value.string, TYPE_UNKNOWN);
                            if(!id_node) { *error_code = ERR_INTERNAL; ast_free(node);
                              goto cleanup;
                            }
                            ast_add_child(node, id_node);

                            // 2. Zpracujeme parametry
                            // next_token je '(', takze ho spotrebujeme a nacteme dalsi
                            token = get_token();
                            // Check for lexical errors after consuming token
                            if (token.type == ERROR) {
                                *error_code = LEXICAL_ERROR;
                                ast_free(node);
                                goto cleanup;
                            }

                            ASTNode* args_node = params(error_code); // params zacina tokenem PO '('
                            if (*error_code != ERR_OK) { ast_free(node);
                              goto cleanup;
                            }
                            ast_add_child(node, args_node);

                            // 3. Zkontrolujeme ')'
                            // 'params' nam nechal ')' v 'token'
                            if (!check_and_take_token(BRACKET_END, error_code)) {
                                ast_free(node);
                                goto cleanup;
                            }
                            // 'check_and_take_token' spotreboval ')' a nacetl dalsi token do globalni 'token'

                        } else {
                            // Byl to jen ID (promenna)
                            node = ast_create_node(NODE_ID, current_token.value.string, TYPE_UNKNOWN);
                            if (node == NULL) { *error_code = ERR_INTERNAL;
                              goto cleanup;
                            }
                            token = next_token; // Dalsi token uz mame nacteny
                        }

                    } else if (current_token.type == IFJ) {
                        // Je to volani vestavene funkce IFJ
                        fprintf(out, "DEBUG: Zpracovavam IFJ call uvnitr vyrazu\n");
                        node = built_in_call(error_code); // built_in_call spotrebuje IFJ, ., ID, (...), )
                        if (*error_code != ERR_OK) {
                          goto cleanup;
                        }
                        token = get_token(); // Nacteme token *po* ')'
                        // Check for lexical errors after consuming token
                        if (token.type == ERROR) {
                            *error_code = LEXICAL_ERROR;
                            ast_free(node);
                            goto cleanup;
                        }

                    } else {
                        // Je to jiny operand (literal, apod.)
                        node = create_ast_node_from_token(current_token, error_code);
                        if (*error_code != ERR_OK) {
                          goto cleanup;
                        }
                        // create_ast_node_from_token si uz sam nacetl dalsi token
                    }

                    // Push hotovy uzel (nebo podstrom) na AST zasobnik
                    if(ast_stack_push(&astStack, node) != ERR_OK) {
                         *error_code = ERR_INTERNAL;
                         goto cleanup;
                    }

                } else {
                     token = get_token(); // Precti dalsi token (pokud to nebyl operand)
                     // Check for lexical errors after consuming token
                      if (token.type == ERROR) {
                          *error_code = LEXICAL_ERROR;
                          goto cleanup;
                      }
                }
                break; // Konec case S/P/E

            case R: // Reduce '>'
            is_new_token = false; // Po redukci neprecteme novy token
            fprintf(out, "DEBUG: Pravidlo REDUCE >\n");
                if (do_reduction(&tokenStack, &astStack, error_code) != ERR_OK) {
                    goto cleanup;
                }
                // Po redukci necteme novy token!
                break;

            case X: // Error
            default:
                fprintf(out, "ERROR: Chyba v precedencni tabulce. [%d][%d]\n", top_idx, current_idx);
                *error_code = SYNTAX_ERROR;
                goto cleanup;
        }

        if (*error_code != ERR_OK) {
            goto cleanup;
        }

        // Debug vypis zasobniku
        //print_token_stack(&tokenStack);

        // Zkontroluj konec
        stack_top_terminal(&tokenStack, &top_terminal);
        if (top_terminal.type == END_EXPR && token_to_int(token) == IDX_END) {
            success = true;
            break;
        }

    } while (true);

// Stack cleanup and return final AST node
cleanup:
    stack_destroy(&tokenStack);

    if (success) {
        // Na vrcholu astStack by mel byt jediny uzel - koren celeho vyrazu
        if (ast_stack_pop(&astStack, &final_node) == ERR_OK && ast_stack_is_empty(&astStack)) {
            // Hotovo, vracime final_node
            fprintf(out, "___________\n parse_expression OK return \n_____________\n");
        } else {
            fprintf(out, "ERROR: astStack neni prazdny na konci analyzy.\n");
            *error_code = SYNTAX_ERROR;
            final_node = NULL; // Pro jistotu
        }
    } else {
         fprintf(out, "ERROR: Precedencni analyza selhala.\n");
         // Uvolni vsechny uzly, ktere zbyly na zasobniku
         ASTNode* temp_node;
         while(ast_stack_pop(&astStack, &temp_node) == ERR_OK) {
            ast_free(temp_node); // Meli bychom je uvolnit
         }
    }

    ast_stack_destroy(&astStack);
    return final_node;
}


// parsujeme seznam paramteru pri deklaraci funkce
// <PARAM_LIST> --> ID (,ID)*
ASTNode* param_list(int* error_code) {
  fprintf(out, "nasli sme token v param_list: ");
  print_token(token);
  fprintf(out, "\n");
  *error_code = ERR_OK;

  // Check for lexical errors before starting
  if (token.type == ERROR) {
      *error_code = LEXICAL_ERROR;
      return NULL;
  }

  ASTNode* param_list_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
  if (param_list_node == NULL) {
      *error_code = ERR_INTERNAL;
      return NULL;
  }

  if (token.type == BRACKET_END) {
      // prazdny seznam parametru
      fprintf(out, "___________\n param_list OK return \n_____________\n");
      return param_list_node;
  }

  // zpracovat parametry ve while
  while (true) {
      // musi byt ID
      if (token.type != ID) {
          fprintf(out, "ERROR: Ocekavane ID v seznamu parametru, nasli sme: ");
          print_token(token);
          fprintf(out, "\n");
          ast_free(param_list_node);
          *error_code = SYNTAX_ERROR;
          return NULL;
      }

      // vytvorit uzel pro ID
      ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
      if (id_node == NULL) {
          ast_free(param_list_node);
          *error_code = ERR_INTERNAL;
          return NULL;
      }

      // pridat id jako child k param_list_node
      ast_add_child(param_list_node, id_node);

      // nacist dalsi token
      token = get_token();
      // Check for lexical errors after consuming token
      if (token.type == ERROR) {
          *error_code = LEXICAL_ERROR;
          ast_free(param_list_node);
          return NULL;
      }

      if (token.type == COMMA) {
          // dalsi parametr
          token = get_token();
          // Check for lexical errors after consuming token
          if (token.type == ERROR) {
              *error_code = LEXICAL_ERROR;
              ast_free(param_list_node);
              return NULL;
          }

          // zustala carka bez dalsiho parametru
          if (token.type == BRACKET_END) {
              fprintf(out, "ERROR: Zbyla carka bez dalsiho parametru\n");
              ast_free(param_list_node);
              *error_code = SYNTAX_ERROR;
              return NULL;
          }

      } else if (token.type == BRACKET_END) {
          // konec seznamu parametru
          break;
      } else {
          // neocekavany token
          fprintf(out, "ERROR: Neocekavany token v seznamu parametru: ");
          print_token(token);
          fprintf(out, "\n");
          ast_free(param_list_node);
          *error_code = SYNTAX_ERROR;
          return NULL;
      }
  }
  return param_list_node;
}

ASTNode* params(int* error_code) {
    fprintf(out, "nasli sme token v params: ");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    ASTNode* params_node = ast_create_node(NODE_ARG_LIST, NULL, TYPE_UNKNOWN);
    if (params_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // parametry nebudou, takze ), vratim prazdny params_node
    if (token.type == BRACKET_END) {
        fprintf(out, "___________\n params OK return \n_____________\n");
        return params_node;
    }

    // zpracovat parametry ve whilu
    while (true) {
        ASTNode* arg_node = parse_expression(error_code);

        // zkontrolovat chyby pri tvorbe argumentu
        if (*error_code != ERR_OK) {
            ast_free(params_node);
            ast_free(arg_node);
            return NULL;
        }

        // pridat argument jako child k params_node
        if (arg_node != NULL) {
            ast_add_child(params_node, arg_node);
        }

        // nacist dalsi token, pokud je carka
        if (token.type == COMMA) {
            token = get_token();
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                ast_free(params_node);
                return NULL;
            }

            // zustala carka bez dalsiho parametru
            if (token.type == BRACKET_END) {
                fprintf(out, "ERROR: Zbyla carka bez dalsiho parametru\n");
                ast_free(params_node);
                *error_code = SYNTAX_ERROR;
                return NULL;
            }
            // dalsi parametr
            continue;
          } else if (token.type == BRACKET_END) {
            // konec seznamu parametru
            break;
          } else {
            // neocekavany token
            fprintf(out, "ERROR: Neocekavany token v seznamu parametru: ");
            print_token(token);
            fprintf(out, "\n");
            ast_free(params_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
          }
    }

    // vsechno v pohode
    fprintf(out, "___________\n params OK return \n_____________\n");
    *error_code = ERR_OK;
    return params_node;
}


ASTNode* block(int* error_code) {
    fprintf(out, "nasli sme token v block: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BLOCK> -> <{> <NEWLINE> <COMMANDS> <}> <ELSE>*/

    // program() zere prvni {
    // block() chce command dokud neskonci }

    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    *error_code = ERR_OK;
    ASTNode* block_node = ast_create_node(NODE_BLOCK, NULL, TYPE_UNKNOWN);
    if (block_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // poresit optional newline po {
    if (token.type != NEW_LINE) {
        // nemuze byt prazdny blok {}
        if (token.type == BLOCK_END) {
            fprintf(out, "ERROR: Prazdny blok neni povoleny\n");
            ast_free(block_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
        }

        fprintf(out, "Neni NEW_LINE po BLOCK_START\n");
        *error_code = SYNTAX_ERROR;
        ast_free(block_node);
        return NULL;
    }
    // nacist dalsi token po newline
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        ast_free(block_node);
        return NULL;
    }

    // zpracovat prikazy v bloku, dokud neskonci }
    while (token.type != BLOCK_END) {
        int cmd_error = ERR_OK;
        ASTNode* cmd_node = command(&cmd_error);

        if (cmd_error != ERR_OK) {
            *error_code = cmd_error;
            ast_free(block_node);
            return NULL;
        }
        // command() vraci NULL pro newline
        if (cmd_node != NULL) {
            ast_add_child(block_node, cmd_node);
        }

        // EOF nastane pred koncem bloku
        if (token.type == EOF_TOKEN) {
            *error_code = SYNTAX_ERROR;
            ast_free(block_node);
            return NULL;
        }
        // Check for lexical errors after consuming token
        if (token.type == ERROR) {
            *error_code = LEXICAL_ERROR;
            ast_free(block_node);
            return NULL;
        }
    }

    // konec bloku
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        ast_free(block_node);
        return NULL;
    }

    fprintf(out, "___________\n block OK return \n_____________\n");
    return block_node;
}


ASTNode* func_call(int *error_code) {
    fprintf(out, "nasli sme token v func_call: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <FUNC_CALL> -> ID <(>  <PARAMS>  <)>*/

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // pokud neni ID, chyba
    if (token.type != ID) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // vytvorit uzel pro volani funkce
    ASTNode* call_node = ast_create_node(NODE_CALL, NULL, TYPE_UNKNOWN);

    if (call_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // ulozit jmeno funkce jako identifikator
    ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);

    if (id_node == NULL) {
        ast_free(call_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // pridat id jako child ke call node
    ast_add_child(call_node, id_node);

    // nacist dalsi token, musi byt (
    if (!match_token(BRACKET_START)) {
        // Check if match_token failed due to lexical error
        if (token.type == ERROR){
            *error_code = LEXICAL_ERROR;
        } else {
            *error_code = SYNTAX_ERROR;
        }
        ast_free(call_node);
        return NULL;
    }

    // zpracovat parametry
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        ast_free(call_node);
        return NULL;
    }

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);

    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    ast_add_child(call_node, args_node);

    // nacist dalsi token, musi byt )
    if (token.type != BRACKET_END) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    fprintf(out, "___________\n func_call OK return \n_____________\n");
    return call_node;
}

ASTNode* built_in_call(int* error_code) {
    fprintf(out, "nasli sme token v b_i_c: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BUILT_IN_CALL> -> <IFJ> <.> <KW> <(> <PARAMS> <)> */

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // pokud neni IFJ, chyba
    if (token.type != IFJ) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt .
    if (!match_token(DOT)) {
        // Check if match_token failed due to lexical error
        if (token.type == ERROR){
            *error_code = LEXICAL_ERROR;
        } else {
            *error_code = SYNTAX_ERROR;
        }
        return NULL;
    }

    // nacist dalsi token, musi byt id funkce
    if (!match_token(ID) || !is_built_in_func()) {
        // Check if match_token failed due to lexical error
        if (token.type == ERROR){
            *error_code = LEXICAL_ERROR;
        } else {
            *error_code = SYNTAX_ERROR;
        }
        return NULL;
    }

    // potrebuju ulozit jmeno funkce
    char* func_name_only = token.value.string;
    char full_name[strlen(func_name_only) + 5]; // ifj. + \0
    sprintf(full_name, "Ifj.%s", func_name_only);
    char* func_name = str_dup(full_name);

    if (func_name == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // vytvorit uzel pro volani funkce
    ASTNode* call_node = ast_create_node(NODE_CALL, NULL, TYPE_UNKNOWN);
    if (call_node == NULL) {
        free(func_name);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // ulozit jmeno funkce jako id
    ASTNode* id_node = ast_create_node(NODE_ID, func_name, TYPE_UNKNOWN);
    free(func_name);
    if (id_node == NULL) {
        ast_free(call_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // pridat id jako child ke call_node
    ast_add_child(call_node, id_node);
    // nacist dalsi token, musi byt (
    if (!match_token(BRACKET_START)) {
        // Check if match_token failed due to lexical error
        if (token.type == ERROR){
            *error_code = LEXICAL_ERROR;
        } else {
            *error_code = SYNTAX_ERROR;
        }
        ast_free(call_node);
        return NULL;
    }

    // zpracovat parametry
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        ast_free(call_node);
        return NULL;
    }

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);
    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    ast_add_child(call_node, args_node);

    // nacist dalsi token, musi byt )
    if (token.type != BRACKET_END) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    fprintf(out, "___________\n b_i_c OK return \n_____________\n");
    return call_node;
}

ASTNode* cond_loop(int* error_code) {
    fprintf(out, "nasli sme token v cond_loop cond_state: %s: ", (cond_state)? "while":"if");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // kontrola jestli je to if nebo while
    bool is_if = false;
    // vytvorit uzel pro podminku nebo cyklus
    ASTNode *cond_node = NULL;
    // vytvorit parent uzel pro pridani podminky/cyklu
    if (token.type == IF) {
        cond_node = ast_create_node(NODE_IF, NULL, TYPE_UNKNOWN);
        is_if = true;
    } else if (token.type == WHILE) {
        cond_node = ast_create_node(NODE_WHILE, NULL, TYPE_UNKNOWN);
    } else {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    if (cond_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // nacist dalsi token, musi byt (
    token = get_token();
    if (!check_and_take_token(BRACKET_START, error_code)) {
        ast_free(cond_node);
        return NULL;
    }

    // zpracovat vyraz podminky
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(cond_node);
        return NULL;
    }
    ast_add_child(cond_node, expr_node);

    // dalsi token musi byt )
    if (!check_and_take_token(BRACKET_END, error_code)) {
        ast_free(cond_node);
        return NULL;
    }

    // dalsi token, musi byt {
    if (!check_and_take_token(BLOCK_START, error_code)) {
        ast_free(cond_node);
        return NULL;
    }

    // zpracovat blok prikazu
    ASTNode* block_node = block(error_code);
    if (*error_code != ERR_OK) {
        ast_free(cond_node);
        return NULL;
    }
    ast_add_child(cond_node, block_node);

    if (is_if) {
        if (token.type == ELSE) {
            token = get_token();
            // check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                ast_free(cond_node);
                return NULL;
            }

            if (token.type == IF) {
                // else if
                ASTNode* else_if_node = cond_loop(error_code); // rekurzivni zavolam
                if (*error_code != ERR_OK) {
                    ast_free(cond_node);
                    return NULL;
                }
                ast_add_child(cond_node, else_if_node);
            } else if (token.type == BLOCK_START) {
                // klasika else
                token = get_token(); // nacist {
                // check for lexical errors after consuming token
                if (token.type == ERROR) {
                    *error_code = LEXICAL_ERROR;
                    ast_free(cond_node);
                    return NULL;
                }

                ASTNode* else_block_node = block(error_code);
                if (*error_code != ERR_OK) {
                    ast_free(cond_node);
                    return NULL;
                }
                ast_add_child(cond_node, else_block_node);
            } else {
                // chyba
                fprintf(out, "ERROR: Expected IF or BLOCK after ELSE\n");
                ast_free(cond_node);
                *error_code = SYNTAX_ERROR;
                return NULL;
            }
        } else {
            // else neni, vsechno ok
            // pridat prazdny else do AST
            ASTNode* empty_else_node = ast_create_node(NODE_BLOCK, NULL, TYPE_UNKNOWN);
            if (empty_else_node == NULL) {
                ast_free(cond_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            ast_add_child(cond_node, empty_else_node);
        }
    }

    // cekam newline po celym if/while
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(cond_node);
        return NULL;
    }

    fprintf(out, "___________\n cond_loop OK return \n_____________\n");
    return cond_node;
}


ASTNode* assign(int *error_code) { /* TODO poriadne otestovat nove riadky kde mozu a nemozu byt */
    fprintf(out, "nasli sme token v assign: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <ASSIGN> -> <ID> <=> <LITERAL> (or) <EXPRESSION> */

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // vytvorit uzel pro prirazeni
    ASTNode* assign_node = ast_create_node(NODE_ASSIGN, NULL, TYPE_UNKNOWN);
    if (assign_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // musi byt ID nebo GLOBAL_ID
    if ((token.type != ID) && (token.type != GLOBAL_ID)) {
        ast_free(assign_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
}

    // vytvorit uzel pro ID
    ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
    if (id_node == NULL) {
        ast_free(assign_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }
    // pridat id jako child k assign_node
    ast_add_child(assign_node, id_node);

    // nacist dalsi token, musi byt =
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        ast_free(assign_node);
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt literal nebo expression
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(assign_node);
        return NULL;
    }

    // pridat expr_node jako child k assign_node
    ast_add_child(assign_node, expr_node);

    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(assign_node);
        return NULL;
    }

    fprintf(out, "___________\n assign OK return \n_____________\n");
    return assign_node;
}


ASTNode* func_decl(int *error_code) {
    fprintf(out, "nasli sme token v func_decl: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE:  <FUNC_DECL> -> <STATIC> <ID> <=?> <BRACKETS> <BLOCK> */

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // token je STATIC
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    // musi byt ID
    if (token.type != ID) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }
    // potrebuju ulozit nazov funkce
    char *func_name = str_dup(token.value.string);
    if (func_name == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // dalsi token musi byt (
    token = get_token();
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        free(func_name);
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    ASTNode* func_node = NULL;

    if (token.type == BRACKET_START) {
        // vytvorit uzel pro deklaraci funkce, setter nebo getter
        func_node = ast_create_node(NODE_FUNCTION, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        // zpracovat seznam parametru
        token = get_token();
        // Check for lexical errors after consuming token
        if (token.type == ERROR) {
            free(func_name);
            *error_code = LEXICAL_ERROR;
            ast_free(func_node);
            return NULL;
        }

        ASTNode* params_node = param_list(error_code);
        if (*error_code != ERR_OK) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }
        // pridat params_node jako child k func_node
        ast_add_child(func_node, params_node);

        // dalsi token musi byt )
        if (!check_and_take_token(BRACKET_END, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }

    } else if (token.type == EQUAL) {
        // vytvorit uzel pro deklaraci setter funkce
        func_node = ast_create_node(NODE_SETTER, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        token = get_token(); // musi byt =
        // Check for lexical errors after consuming token
        if (token.type == ERROR) {
            free(func_name);
            ast_free(func_node);
            *error_code = LEXICAL_ERROR;
            return NULL;
        }

        if (!check_and_take_token(BRACKET_START, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }

        // setter musi mit jeden parameter
        ASTNode* params_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
        if (params_node == NULL) {
            free(func_name);
            ast_free(func_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        // musi byt ID
        if (token.type != ID) {
            free(func_name);
            ast_free(func_node);
            ast_free(params_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
        }
        // vytvorit uzel pro ID
        ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
        if (id_node == NULL) {
            free(func_name);
            ast_free(func_node);
            ast_free(params_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        // pridat id jako child k params_node
        ast_add_child(params_node, id_node);
        // pridat params_node jako child k func_node
        ast_add_child(func_node, params_node);

        token= get_token(); // nacist dalsi token
        // Check for lexical errors after consuming token
        if (token.type == ERROR) {
            free(func_name);
            ast_free(func_node);
            *error_code = LEXICAL_ERROR;
            return NULL;
        }

        if (!check_and_take_token(BRACKET_END, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }
      } else if (token.type == BLOCK_START) {
        // vytvorit uzel pro deklaraci getter funkce
        func_node = ast_create_node(NODE_GETTER, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        // getter nema parametry, pridat prazdny param_list
        ASTNode* params_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
        if (params_node == NULL) {
            free(func_name);
            ast_free(func_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        ast_add_child(func_node, params_node);
        // nic vic cist, pokracujem na block
    } else {
        free(func_name);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }
    // zpracovat blok prikazu
    free(func_name);
    if (!check_and_take_token(BLOCK_START, error_code)) {
        ast_free(func_node);
        return NULL;
    }

    ASTNode* block_node = block(error_code);
    if (*error_code != ERR_OK) {
        ast_free(func_node);
        return NULL;
    }
    ast_add_child(func_node, block_node);

    // cekam newline po celem func_decl
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(func_node);
        return NULL;
    }

    fprintf(out, "___________\n func_decl OK return \n_____________\n");
    return func_node;
}

ASTNode* return_func(int *error_code) {
    fprintf(out, "nasli sme token v return_func: ");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // token je RETURN

    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    ASTNode* return_node = ast_create_node(NODE_RETURN, NULL, TYPE_UNKNOWN);
    if (return_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // zkontrolovat return bez vyrazu
    if (token.type == NEW_LINE) {
        // pro semantiku return null
        ASTNode* null_node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
        if (null_node == NULL) {
            ast_free(return_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        ast_add_child(return_node, null_node);
        fprintf(out, "___________\n return_func OK return \n_____________\n");
        token= get_token(); // nacist dalsi token
        // Check for lexical errors after consuming token
        if (token.type == ERROR) {
            ast_free(return_node);
            *error_code = LEXICAL_ERROR;
            return NULL;
        }
        return return_node;
    }

    // zpracovat vyraz za return
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(return_node);
        return NULL;
    }
    ast_add_child(return_node, expr_node);
    // cekam newline po return
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(return_node);
        return NULL;
    }

    fprintf(out, "___________\n return_func OK return \n_____________\n");
    return return_node;
}


ASTNode* command(int *error_code) {
    fprintf(out, "nasli sme token v command: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <COMMAND> -> <FUNC_CALL> <NEW_LINE>
                      | <ASSIGN> <NEW_LINE>
                      | <BUILT_IN_CALL> <NEW_LINE>
                      | <RETURN> <NEW_LINE>
                      | <VAR> <ID> <NEW_LINE>
                      | <COND_LOOP>
                      | <FUNC_DECL>
                      | <NEW_LINE>
                      | <BLOCK_END>
    */

    *error_code = ERR_OK;
    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    ASTNode* command_node = NULL;

    switch(token.type){
        case ID: {
          Token id_token = token; // Ulozim si token id
          token = get_token(); // Nactu dalsi token
          // Check for lexical errors after consuming token
          if (token.type == ERROR) {
              *error_code = LEXICAL_ERROR;
              return NULL;
          }

          if (token.type == BRACKET_START) {
              // Je to volani funkce
              token = id_token; // Vratim se k id tokenu pro func_call
              command_node = func_call(error_code);
              if (*error_code != ERR_OK) {
                  return NULL;
              }

              // func_call necha token )
              // cekam NEW_LINE
              if (!check_and_take_token(BRACKET_END, error_code)) {
                  ast_free(command_node);
                  return NULL;
              }

              if (!check_and_take_token(NEW_LINE, error_code)) {
                  ast_free(command_node);
                  return NULL;
              }

          } else if (token.type == EQUAL) {
              // Neni to volani, takze prirazeni
              token = id_token; // Vratim se k id tokenu pro assign
              command_node = assign(error_code);
              if (*error_code != ERR_OK) {
                  return NULL;
              }
            // assign necha token za vyrazem
          } else {
              fprintf(out, "ERROR: Neocekavany token po ID: ");
              print_token(token);
              fprintf(out, "\n");
              *error_code = SYNTAX_ERROR;
              return NULL;
          }
          break;
        }


        case IFJ:
            command_node = built_in_call(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // built_in_call necha token )
            if (!check_and_take_token(BRACKET_END, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            // cekam NEW_LINE
            if (!check_and_take_token(NEW_LINE, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            break;

        case RETURN:
            token = get_token(); // vezmu dalsi token
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                return NULL;
            }

            command_node = return_func(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // uz vzal newline
            break;

        case GLOBAL_ID: {
          Token id_token = token; // Ulozime GLOBAL_ID
            token = get_token(); // Nacteme dalsi token (musi byt EQUAL)
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                return NULL;
            }

            if (token.type == EQUAL) {
                // Je to prirazeni
                token = id_token; // Vratime token zpet na GLOBAL_ID
                command_node = assign(error_code);
                if (*error_code != ERR_OK){ return NULL; }
                // assign uz se postaral o newline
            } else {
                fprintf(out, "ERROR: Neocekavany token po GLOBAL_ID: ");
                print_token(token);
                fprintf(out, "\n");
                *error_code = SYNTAX_ERROR;
                return NULL;
            }
            break;
        }


        case VAR: {
            // deklarace promenne
            token = get_token(); // nacist dalsi token, musi byt ID
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                return NULL;
            }

            if (token.type != ID) {
                *error_code = SYNTAX_ERROR;
                return NULL;
            }

            // ulozit jmeno promenne
            char* var_name = str_dup(token.value.string);
            if (var_name == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }

            // vytvorit uzel pro deklaraci promenne
            command_node = ast_create_node(NODE_ASSIGN, NULL, TYPE_UNKNOWN);
            if (command_node == NULL) {
                *error_code = ERR_INTERNAL;
                free(var_name);
                return NULL;
            }

            // vytvorit uzel pro ID
            ASTNode* id_node = ast_create_node(NODE_ID, var_name, TYPE_UNKNOWN);
            free(var_name);
            if (id_node == NULL) {
                ast_free(command_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            // pridat id jako child k assign_node
            ast_add_child(command_node, id_node);

            // promenna inicializovana na null
            ASTNode* null_node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
            if (null_node == NULL) {
                ast_free(command_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            // pridat null jako child k assign_node
            ast_add_child(command_node, null_node);
            // cekam NEW_LINE
            token = get_token();
            if (!check_and_take_token(NEW_LINE, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            break;
        }

        case IF:
        case WHILE:
            command_node = cond_loop(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // cond_loop uz vzal newline
            break;

        case STATIC:
            command_node = func_decl(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // func_decl uz vzal newline
            break;

        case NEW_LINE:
            token = get_token();
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                return NULL;
            }
            return NULL; // vratim NULL a nepridam do AST
            break;

        case BLOCK_END:
            fprintf(out, "___________\n command OK return \n_____________\n");
            return NULL; // vratim NULL a nepridam do AST
            break;

        case BLOCK_START:
            // Samostatny blok jako prikaz
            // 1. Spotrebujeme '{'
            token = get_token();
            // Check for lexical errors after consuming token
            if (token.type == ERROR) {
                *error_code = LEXICAL_ERROR;
                return NULL;
            }

            // 2. Zavolame 'block', ten ocekava NEW_LINE jako dalsi token
            command_node = block(error_code);
            if (*error_code != ERR_OK) {
                return NULL;
            }

            // 3. 'block' nam nechal token *po* '}'
            // Ocekavame, ze to bude NEW_LINE, stejne jako po jinem prikazu
            if (!check_and_take_token(NEW_LINE, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            break;

        default:
            *error_code = SYNTAX_ERROR;
            return NULL;
            break;
   }

   if (*error_code != ERR_OK) {
        ast_free(command_node);
        return NULL;
   }

  return command_node;
}

int valid(){
    fprintf(out, "sme v valid\n");
    fprintf(out, "nasli sme token v valid: ");
    print_token(token);
    fprintf(out, "\n");

    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        return LEXICAL_ERROR;
    }

    switch(token.type){
        case IMPORT:
            if (!match_token(STRING)) {
                if (token.type == ERROR) {
                    return LEXICAL_ERROR;
                }
                if (token.type != NEW_LINE) {
                    return SYNTAX_ERROR;
                }
            }
            return valid();
            break;

        case STRING:
            if (strcmp(token.value.string, "ifj25") == 0){
                if (!match_token(FOR)) {
                    if (token.type == ERROR) {
                        return LEXICAL_ERROR;
                    }
                    return SYNTAX_ERROR;
                }
                return valid();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case FOR:
            if (!match_token(IFJ) && (token.type != NEW_LINE)) {
                if (token.type == ERROR) {
                    return LEXICAL_ERROR;
                }
                return SYNTAX_ERROR;
            }
            return valid();
            break;

        case IFJ:
            if (match_token(NEW_LINE)){
                // musim mit novy token, protoze match_token() posunul na NEW_LINE
                token = get_token();
                // Check for lexical errors after consuming token
                if (token.type == ERROR) {
                    return LEXICAL_ERROR;
                }
                return ERR_OK;
            }
            if (token.type == ERROR) {
                return LEXICAL_ERROR;
            }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            token = get_token();
            return valid(); // recursion checks error codes
            break;

        default:
            return SYNTAX_ERROR; // invalid start token
            break;
    }
    return SYNTAX_ERROR;
}

ASTNode* program(int* error_code){
    fprintf(out, "nasli sme token v prog: ");
    print_token(token);
    fprintf(out, "\n");

    // Check for lexical errors before starting
    if (token.type == ERROR) {
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    if (token.type != CLASS) {
      fprintf(out, "ERROR: Musi byt 'class'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    if (!match_token(ID) || strcmp(token.value.string, "Program") != 0) {
      fprintf(out, "ERROR: Musi byt 'Program'\n");
      if (token.type == ERROR) {
          *error_code = LEXICAL_ERROR;
      } else {
          *error_code = SYNTAX_ERROR;
      }
      return NULL;
    }

    if (!match_token(BLOCK_START)) {
      fprintf(out, "ERROR: Musi byt '{'\n");
      if (token.type == ERROR) {
          *error_code = LEXICAL_ERROR;
      } else {
          *error_code = SYNTAX_ERROR;
      }
      return NULL;
    }

    ASTNode* program_node = ast_create_node(NODE_PROGRAM, NULL, TYPE_UNKNOWN);

    if (!program_node) {
      fprintf(out, "ERROR: Failed to create AST node\n");
      *error_code = ERR_INTERNAL;
      return NULL;
    }

    token = get_token(); // Move to the next token after '{'
    // Check for lexical errors after consuming token
    if (token.type == ERROR) {
        ast_free(program_node);
        *error_code = LEXICAL_ERROR;
        return NULL;
    }

    int child_error = ERR_OK;
    ASTNode* block_node = block(&child_error); // Parse the block

    if (child_error != ERR_OK) {
      fprintf(out, "ERROR: Failed to parse block\n");
      ast_free(program_node);
      *error_code = child_error;
      return NULL;
    }

    // block() vzal '}'
    ast_add_child(program_node, block_node);

    // spotrebovat vsechny NEW_LINE po programu
    while (token.type == NEW_LINE) {
      token = get_token();
      // Check for lexical errors after consuming token
      if (token.type == ERROR) {
          ast_free(program_node);
          *error_code = LEXICAL_ERROR;
          return NULL;
      }
    }

    if (token.type != EOF_TOKEN) {
      fprintf(out, "ERROR: Expected end of file but found %s\n", token_type_to_string(token.type));
      ast_free(program_node);
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    *error_code = ERR_OK;
    return program_node;
}

ASTNode* run_parser(FILE* input, FILE* output, int* parse_error) {
    // Initialize global file pointers
    in = input;
    out = output;

    // Initialize the scanner
    scanner_init(in, out);
    // Get the first token
    token = get_token();

    ASTNode* ast_root = NULL;
    int error_code = ERR_OK;

    // Validate the program structure
    int valid_error = valid();
    if (valid_error != ERR_OK) {
        if (valid_error == LEXICAL_ERROR) {
            fprintf(out, "VALIDATION LEXICAL ERROR of prolog (import...)\n");
            error_code = LEXICAL_ERROR;
        } else {
            fprintf(out, "VALIDATION SYNTAX ERROR of prolog (import...)\n");
            error_code = SYNTAX_ERROR;
        }
    } else {
        // Parse the main program
        ast_root = program(&error_code);
    }

    // Set the parse error code
    if (error_code != ERR_OK) {
        if (ast_root != NULL) {
            ast_free(ast_root);
        }
        *parse_error = error_code; // Propagate the error code
        return NULL;
    }

    // No errors, return the AST root
    *parse_error = ERR_OK;
    return ast_root;
}

// ===============================================================================================================
// MAIN FUNCTION FOR TESTING PARSER ONLY
// ===============================================================================================================

// int main (int argc, char** argv){
//     (void)argc;
//     (void)argv;

//     in = fopen("../samples/ex0-vsechny-konstrukce.wren", "r");
//     if (!in){ printf("nejde otvorit\n"); return ERR_INTERNAL; }

//     out = fopen("../samples/outfile.txt", "w");
//     if (!out){ return ERR_INTERNAL; }

//     scanner_init(in, out);

//     int ok;
//     // ok = 1;

//     token = get_token();

//     ASTNode* ast_root = NULL;
//     int parse_error = ERR_OK;


//     if (valid() == ERR_OK){
//         fprintf(out, "VALID OK\n");

//         ast_root = program(&parse_error);

//         if (parse_error == ERR_OK){
//             ok = 1;
//             fprintf(out, "\n<AST representation>\n");
//             ast_fprint_debug(ast_root, out);
//         } else {
//             ok = 0;
//             fprintf(out, "PARSE ERROR\n");
//         }
//     } else {
//         fprintf(out, "VALID NOT OK\n");
//         parse_error = SYNTAX_ERROR;
//     }

//     // uvolnit AST
//     ast_free(ast_root);
//     fprintf(out, "Program ended %s\n", (ok == 1) ? "successfully" : "with error");
//     fclose(in);
//     fclose(out);


//     return (ok == 1) ? ERR_OK : parse_error;
// }
