/**
 * @file parser.c
 * @author Šimon Čorej (xcorejs00)
 * @brief Main Parser Function
 * @version 0.1
 * @date 2025-10-12
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"


// TODO: vlozit do expression() moznost kontroly spravnosti (a+5 == 10)
// TODO: doplnit moznost ! v parametroch expression ternary atd
// TODO: doplnit do funkcii assign params return case OPERATOR: return expression
// TODO: doplnit do tych istych funkcii case QUESTION: return ternary()
//      -> ? nesmie byt prvy operator a kontrola ci token pred nim je spravnej hodnoty bude na semantike
//      -> kontrola a : b bude na ternary()
//      -> ternary() bude kontrolovat iba ( ) ? :
//      -> to co bolo pred ? ci je spravne napisane skontroluje dana funkcia ostatne je na ternary()

FILE* in;
FILE* out;

/* condition state 0 = IF 1 = WHILE */
bool cond_state = 0;

TokenType list_param_ops[] = {IS, EQUAL_EQUAL, LESS, LESS_EQUAL, MORE, MORE_EQUAL, NOT, NOT_EQUAL, AND, OR};
char* built_in_string[] = {"read_str", "read_num", "write", "floor", "str", "length", "substring", "strcmp", "ord", "chr", "read_bool"};

bool match_token(TokenType type){
    token = get_token();
    return (token.type == type);
}

bool is_built_in_func(){
    for (int i = 0; i < 11; i++){
        if (strcmp(token.value.string, built_in_string[i]) == 0){ return true; }
    }
    return false;
}

// bool is_operator(){
//     return ((token.type == PLUS) || (token.type == MINUS) || (token.type == MULTIPLY) || (token.type == DIVIDE));
// }

bool is_operand(){
    return ((token.type == ID) || (token.type == GLOBAL_ID) || (token.type == STRING)
            || (token.type == FLOATING) || (token.type == BOOLEAN) || (token.type == INTEGER));
}

bool is_param_expr(){
    for (int i = 0; i < 10; i++){
        if (list_param_ops[i] == token.type){ return true; }
    }
    return false;
}

int expression(){ /* pozor pri assign konci az po nacitani ")" chyba, a = a + 5*/
    while(token.type != BRACKET_END){
        token = get_token();
    }
    // token = get_token();
    fprintf(out, "___________\n expression OK return \n_____________\n");
    return ERR_OK;
}

// int ternary_arg_check(){
//     switch(token.type){

//     }
// }

int ternary(){ /*mozno do buducna vytvorit <before> : <after> ktore by kontorolovali jednotlive expressions*/
    fprintf(out, "nasli sme token v ternary: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case BRACKET_START:
            if (!match_token(BRACKET_END)){ return ternary(); }
            return SYNTAX_ERROR;
            break;

        case BRACKET_END:
            if (!match_token(BRACKET_START)){ return ternary(); }
            return SYNTAX_ERROR;
            break;


        case ID: /* neskor doplnit ... ? ID? y : n : n */
            if (match_token(BRACKET_START)){
                if ((func_call() == ERR_OK) && (match_token(COLON) || (token.type == OPERATOR) || (token.type == NEW_LINE) || (token.type == BRACKET_END))){
                    return ternary();
                }
            } else if (token.type == OPERATOR){
                if ((expression() == ERR_OK) && (match_token(COLON) || (token.type == NEW_LINE))){
                    return ternary();
                }
            } else if (token.type == NEW_LINE){ return ternary(); }
            return SYNTAX_ERROR;
            break;

        case GLOBAL_ID:
        case STRING:
        case INTEGER:
        case FLOATING:
            if (match_token(OPERATOR)){
                if ((expression() == ERR_OK) && (match_token(COLON) || (token.type == NEW_LINE))){ return ternary(); }
            } else if ((token.type == COLON) || (token.type == NEW_LINE)){ return ternary(); }

            return SYNTAX_ERROR;
            break;

        case BOOLEAN:
            if (match_token(COLON) || (token.type == NEW_LINE)){ return ternary(); }
            return SYNTAX_ERROR;
            break;

        case OPERATOR:
            if (expression() == ERR_OK){
                token = get_token();
                return ternary();
            }

            return SYNTAX_ERROR;
            break;

        case COLON:
        case QUESTION:
            token = get_token();
            return ternary();

        case NEW_LINE:
            fprintf(out, "___________\n ternary OK return\n_____________\n");
            return ERR_OK;
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

int params(){
    fprintf(out, "nasli sme token v params: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case BRACKET_START:
            if (match_token(COMMA)){ return SYNTAX_ERROR; }
            return params();
            break;

        case IFJ:
            if ((built_in_call() == ERR_OK) && (match_token(BRACKET_END))){ return params(); }
            break;

        case ID:
            if (match_token(BRACKET_START)){
                if ((func_call() == ERR_OK) && (match_token(BRACKET_END))){ return params(); }

            } else if (is_param_expr()){
                token = get_token();
                return params();

            } else if (token.type == OPERATOR){
                if (expression() == ERR_OK){ return params(); }
            }
            return params();
            break;

        case STRING:
        case INTEGER:
        case FLOATING:
        case GLOBAL_ID:
        case BOOLEAN:
            if ((!match_token(BRACKET_END)) && (!is_param_expr())){
                if (expression() == ERR_OK){ return params(); }
            } /* tu je chyba pretoze ","*/
            else if (is_param_expr()){
                token = get_token();
            } else if (token.type == OPERATOR){
                if (expression() == ERR_OK){ token = get_token(); return params(); }
            }
            return params();
            break;

        case NUM_TYPE:
        case STR_TYPE:
        case NULL_TYPE:
        case BOOL_TYPE:
        case NULL_KEYWORD:
            if ((!match_token(BRACKET_END)) && (token.type != COMMA)){ return SYNTAX_ERROR; }
            return params();
            break;

        case COMMA:
            if (match_token(BRACKET_END)){ return SYNTAX_ERROR; }
            return params();
            break;

        case BRACKET_END:
            fprintf(out, "___________\n params OK return\n_____________\n");
            return ERR_OK;
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function <BLOCK> on default calls command function
 *
 * @return SYNTAX_ERROR or OK if finished
 */
ASTNode* block(int* error_code) {
    fprintf(out, "nasli sme token v block: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BLOCK> -> <{> <NEWLINE> <COMMANDS> <}> <ELSE>*/

    // program() zere prvni {
    // block() chce command dokud neskonci }

    *error_code = ERR_OK;
    ASTNode* block_node = ast_create_node(NODE_BLOCK, NULL, TYPE_UNKNOWN);
    if (block_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // poresit optional newline po {
    if (token.type == NEW_LINE) {
        token = get_token();
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
    }

    // konec bloku
    token = get_token();
    fprintf(out, "___________\n block OK return \n_____________\n");
    return block_node;
}

/**
 * @brief Function to call user made functions
 *
 * @return Function return ASTNode* of <func_call> or NULL on error
 */
ASTNode* func_call(int *error_code) {
    fprintf(out, "nasli sme token v func_call: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <FUNC_CALL> -> ID <(>  <PARAMS>  <)>*/

    *error_code = ERR_OK;
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
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat parametry
    token = get_token();

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);

    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    args_node->type = NODE_ARG_LIST;
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

/**
 * @brief Function to call built in functions
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* built_in_call(int* error_code) {
    fprintf(out, "nasli sme token v b_i_c: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BUILT_IN_CALL> -> <IFJ> <.> <KW> <(> <PARAMS> <)> */

    *error_code = ERR_OK;
    // pokud neni IFJ, chyba
    if (token.type != IFJ) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt .
    if (!match_token(DOT)) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt id funkce
    if (!match_token(ID) || !is_built_in_func()) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // potrebuju ulozit jmeno funkce
    char* func_name = str_dup(token.value.string);
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
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat parametry
    token = get_token();

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);
    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    args_node->type = NODE_ARG_LIST;
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

/**
 * @brief Function to use conditions and while loops
 * @todo doplnit kde mozu byt newlines
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* cond_loop(int* error_code) {
    fprintf(out, "nasli sme token v cond_loop cond_state: %s: ", (cond_state)? "while":"if");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // vytvorit uzel pro podminku nebo cyklus
    ASTNode *cond_node = NULL;
    // vytvorit parent uzel pro pridani podminky/cyklu
    if (token.type == IF) {
        cond_node = ast_create_node(NODE_IF, NULL, TYPE_UNKNOWN);
        cond_state = 0;
    } else if (token.type == WHILE) {
        cond_node = ast_create_node(NODE_WHILE, NULL, TYPE_UNKNOWN);
        cond_state = 1;
    } else {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    if (cond_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // nacist dalsi token, musi byt (
    if (!match_token(BRACKET_START)) {
        ast_free(cond_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat vyraz podminky/cyklu
    token = get_token();

    // TODO: upravit params()


    return parent_node;
}

/**
 * @brief Function to assign value or expression
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
int assign(){ /* TODO poriadne otestovat nove riadky kde mozu a nemozu byt */
    fprintf(out, "nasli sme token v assign: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <ASSIGN> -> <ID> <=> <LITERAL> (or) <EXPRESSION> */
    switch(token.type){
        case EQUAL:
            token = get_token();
            return assign();
            break;

        case ID:
            if (match_token(BRACKET_START)){
                if (func_call() == ERR_OK){
                    token = get_token();
                    return assign();
                }
            } else if ((token.type == NEW_LINE) || (token.type == OPERATOR) || (token.type == QUESTION)){
                return assign();
            }
            return SYNTAX_ERROR;
            break;

        case STRING:
        case INTEGER:
        case FLOATING:
        case GLOBAL_ID:
            if (match_token(NEW_LINE)) { /* declaration tuto pozor na x<NLx> = x<NLx> *<NL*> 5<NL+>*/
                fprintf(out, "___________\n assign OK return \n_____________\n");
                return assign();
            } else if (token.type == OPERATOR){
                return assign();
            }
            return SYNTAX_ERROR;
            break;

        case BOOLEAN:
            if (match_token(NEW_LINE) || (token.type == QUESTION)){ return assign(); }
            return SYNTAX_ERROR;
            break;

        case IFJ:
            if (built_in_call() == ERR_OK){
                fprintf(out, "old token:");
                print_token(token);
                token = get_token();
                fprintf(out, "new token:");
                print_token(token);
                return assign();
            }
            return SYNTAX_ERROR;
            break;

        case OPERATOR:
            if (expression() == ERR_OK){
                token = get_token();
                return assign();
            }
            return SYNTAX_ERROR;
            break;

        case QUESTION:
            if (ternary() == ERR_OK){ fprintf(out, "___________\n assign OK return\n_____________\n"); return ERR_OK; }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            fprintf(out, "___________\n assign OK return\n_____________\n");
            return ERR_OK;
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to declare user made functions
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
int func_decl(){
    fprintf(out, "nasli sme token v func_decl: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE:  <FUNC_DECL> -> <STATIC> <ID> <=?> <BRACKETS> <BLOCK> */
    switch(token.type){
        case STATIC:
            if (!match_token(ID)){ return SYNTAX_ERROR; }
            return func_decl();
            break;

        case ID:
            if (match_token(BRACKET_START)){ /* user made */
                return func_decl();
            } else if (token.type == EQUAL){ /* setter */
                return func_decl();
            } else if (token.type == BLOCK_START){ /* getter */
                return func_decl();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case EQUAL:
            if (!match_token(BRACKET_START)){ return SYNTAX_ERROR; }
            return func_decl();
            break;

        case BRACKET_START:
            if (match_token(BRACKET_END)){
                return func_decl();
            } else if (params() == ERR_OK){
                return func_decl();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case BRACKET_END:
            if (!match_token(BLOCK_START)){ return SYNTAX_ERROR; }
            return func_decl();
            break;

        case BLOCK_START:
            if (block() == ERR_OK){ fprintf(out, "___________\n func_decl OK return \n_____________\n"); return ERR_OK; }
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

int return_func(){
    fprintf(out, "nasli sme token v return_func: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case ID:
            if (match_token(BRACKET_START)){
                if ((func_call() == ERR_OK) && (((match_token(NEW_LINE)) || (token.type == QUESTION)) || (token.type == OPERATOR))){ return return_func(); }
            }
            if (token.type == OPERATOR){ /* expression returns end token either bracket newline or ? */
                if ((expression() == ERR_OK) && (match_token(NEW_LINE) || (token.type == QUESTION))){ return return_func(); }
            }
            if (token.type == QUESTION){
                if ((ternary() == ERR_OK) && ((match_token(NEW_LINE)) || (token.type == OPERATOR))){ return return_func(); }
            }
            if (token.type == NEW_LINE){ return return_func(); }

            return SYNTAX_ERROR;
            break;

        case GLOBAL_ID:
        case STRING:
        case INTEGER:
        case FLOATING:
            if (match_token(OPERATOR)){
                if (expression() == ERR_OK){ token = get_token(); return return_func(); }
            }
            if (token.type == NEW_LINE){ return return_func(); }
            return SYNTAX_ERROR;
            break;

        case BOOLEAN:
            if (match_token(NEW_LINE)){ return return_func(); }
            if (token.type == QUESTION){
                if (ternary() == ERR_OK){ return return_func(); }
            }
            return SYNTAX_ERROR;
            break;

        case IFJ:
            if (built_in_call() == ERR_OK){
                token = get_token();
                return return_func();
            }
            return SYNTAX_ERROR;
            break;

        case NULL_KEYWORD:
            if (match_token(NEW_LINE)){ return return_func(); }
            return SYNTAX_ERROR;
            break;

        case OPERATOR:
            if (expression() == ERR_OK){ token = get_token(); return return_func(); }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            fprintf(out, "___________\n return_func OK return\n_____________\n");
            return ERR_OK;
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
}


int command(){
    fprintf(out, "nasli sme token v command: ");
    print_token(token);
    fprintf(out, "\n");
   switch(token.type){
        case ID:
            if (match_token(BRACKET_START)){
                if ((func_call() == ERR_OK) && (match_token(NEW_LINE))){ return command(); }
            } else if (token.type == EQUAL){
                if (assign() == ERR_OK){ return command(); }
            } else {
                fprintf(out, "ERROR command id else\n");
                return SYNTAX_ERROR;
            }
            return SYNTAX_ERROR;
            break;

        case IFJ:
            if ((built_in_call() == ERR_OK) && (match_token(NEW_LINE))){
                return command();
            }
            return SYNTAX_ERROR;
            break;

        case RETURN:
            if (match_token(NEW_LINE)){ return command(); }
            if (return_func() == ERR_OK){ return command(); }
            return SYNTAX_ERROR;

        case GLOBAL_ID: /* chyba var a global id sa nevolaju rovnako*/
            if (match_token(EQUAL)){
                if (assign() == ERR_OK){ return command(); }
            } else {
                fprintf(out, "globalid or var next token wasnt =\n");
                return SYNTAX_ERROR;
            }
            break;

        case VAR:
            if ((!match_token(ID)) && (token.type != GLOBAL_ID) && (token.type != STRING) && (token.type != INTEGER)
                && (token.type != FLOATING) && (token.type != BOOLEAN)){ return SYNTAX_ERROR; }

            if (match_token(NEW_LINE)){ return command(); }
            return SYNTAX_ERROR;
            break;

        case IF:
        case WHILE:
            if (cond_loop() == ERR_OK){ return command(); }
            return SYNTAX_ERROR;
            break;

        case STATIC:
            if (func_decl() == ERR_OK){ return command(); }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            token = get_token();
            return command();
            break;

        case BLOCK_END:
            fprintf(out, "___________\n command OK return \n_____________\n");
            return ERR_OK; /* no other commands end block*/
            break;

        default:
            return SYNTAX_ERROR;
            break;
   }
   return SYNTAX_ERROR;
}

int valid(){
    fprintf(out, "sme v valid\n");
    fprintf(out, "nasli sme token v valid: ");
    print_token(token);
    fprintf(out, "\n");

    switch(token.type){
        case IMPORT:
            if ((!match_token(STRING)) && (token.type != NEW_LINE)){ return SYNTAX_ERROR; }
            return valid();
            break;

        case STRING:
            if (strcmp(token.value.string, "ifj25") == 0){
                if (!match_token(FOR)){ return SYNTAX_ERROR; }
                return valid();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case FOR:
            if (!match_token(IFJ) && (token.type != NEW_LINE)){ return SYNTAX_ERROR; }
            return valid();
            break;

        case IFJ:
            if (match_token(NEW_LINE)){ return ERR_OK; }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            token = get_token();
            return valid();
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

ASTNode* program(int* error_code){
    fprintf(out, "nasli sme token v prog: ");
    print_token(token);
    fprintf(out, "\n");

    if (token.type != CLASS) {
      fprintf(out, "ERROR: Musi byt 'class'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    if (!match_token(ID) || strcmp(token.value.string, "Program") != 0) {
      fprintf(out, "ERROR: Musi byt 'Program'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    if (!match_token(BLOCK_START)) {
      fprintf(out, "ERROR: Musi byt '{'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    ASTNode* program_node = ast_create_node(NODE_PROGRAM, NULL, TYPE_UNKNOWN);

    if (!program_node) {
      fprintf(out, "ERROR: Failed to create AST node\n");
      *error_code = ERR_INTERNAL;
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

    // block() took '}'
    ast_add_child(program_node, block_node);

    if (token.type != EOF_TOKEN) {
      fprintf(out, "ERROR: Expected end of file\n");
    }

    *error_code = ERR_OK;
    return program_node;
}

// ===========================================DEBUG PRECEDENCE=====================================================
void tok_print(TokenType tok){
    switch(tok){
        case LESS:            fprintf(out, "<  |  "); break;
        case MORE:            fprintf(out, ">  |  "); break;
        case EQUAL:            fprintf(out, "= |  "); break;
        case ERROR:            fprintf(out, "ERR  |   "); break;

        default:
            break;
    }
}
void precedence_check(){
    Token token1;
    Token token2;
    fprintf(out, "table: \n");
    for (int i = 0; i < 43; i++){
        token1 = get_token();

        // if (token1.type == EOF_TOKEN){ break; }

        if (token1.type == NEW_LINE){ fprintf(out, "\n"); continue; }

        token2 = get_token();

        fprintf(out, "%s,", token1.value.string);

        if (token2.type == INTEGER){
            fprintf(out, "%d |  ", token2.value.integer);
        }
        else if (token2.type == FLOATING){
            fprintf(out, "%f |  ", token2.value.floating);
        }
        else if (token2.type == BRACKET_START){
            fprintf(out, "( |  ");
        }
        else if (token2.type == BRACKET_END){
            fprintf(out, ") |  ");
        }
        else {
            fprintf(out, "%s |  ", token2.value.string);
        }

        tok_print(precedence_table[token_to_int(token1)][token_to_int(token2)]);
    }
}
// ===============================================================================================================


int main (int argc, char** argv){
    (void)argc;
    (void)argv;

    in = fopen("../samples/ahoj.IFJcode25", "r");
    if (!in){ printf("nejde otvorit\n"); return ERR_INTERNAL; }

    out = fopen("../samples/outfile.txt", "w");
    if (!out){ return ERR_INTERNAL; }

    scanner_init(in, out);

    int ok;
    // ok = 1;

    token = get_token();

    ASTNode* ast_root = NULL;
    int parse_error = ERR_OK;


    // precedence_check();
    // if (token.type == EQUAL){
    //     Stack stack;
    //     if (stack_init(&stack) != ERR_OK){ return SYNTAX_ERROR; }

    //     stack_push(&stack, END_EXPR);

    //     int value;
    //     token = get_token();
    //     value = expression_val(&stack);
    //     fprintf(out, "calculated value: %d\n", value);
    // }



    if (valid() == ERR_OK){
        fprintf(out, "VALID OK\n");

        ast_root = program(&parse_error);

        if (parse_error == ERR_OK){
            ok = 1;
            fprintf(out, "\n<AST representation>\n");
            ast_print_debug(ast_root, 0);
        } else {
            ok = 0;
            fprintf(out, "PARSE ERROR\n");
        }
    } else {
        fprintf(out, "VALID NOT OK\n");
        parse_error = SYNTAX_ERROR;
    }

    fprintf(out, "Program ended %s\n", (ok == 1) ? "successfully" : "with error");
    fclose(in);
    fclose(out);


    return (ok == 1) ? ERR_OK : parse_error;
}
