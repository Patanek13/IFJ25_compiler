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


// TODO doplnit floating ako lirteral
// mozno vyuzit lookup kde treba check scanner.c

FILE* in;
FILE* out;

Token token;
TokenType list_param_ops[] = {IS, EQUAL_EQUAL, LESS, LESS_EQUAL, MORE, MORE_EQUAL, NOT, NOT_EQUAL, AND, OR};

bool match_token(TokenType type){
    token = get_token();
    return (token.type == type);
}

bool is_operator(){
    return ((token.type == PLUS) || (token.type == MINUS) || (token.type == MULTIPLY)
            || (token.type == DIVIDE));
}

bool is_param_expr(){
    for (int i = 0; i < 10; i++){
        if (list_param_ops[i] == token.type){ return true; }
    }
    return false;
}

int expression(){
    return OK;
}

int params(){  /* dokoncit chyba pri (arg is Num)*/
    fprintf(out, "nasli sme token v params: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case BRACKET_START:
            if (match_token(COMMA)){ return SYNTAX_ERROR; }
            return params();
            break;
        
        case ID:
        case STRING:
        case NUMBER:
        case GLOBAL_ID:
        case BOOLEAN: /* tu este mozno doriesit (ifj.read) alebo (foo(a)) go to expr ak token nie je ) alebo is == != ! ...*/
            fprintf(out, "WEEEE CHECKIN\n");
            if ((!match_token(BRACKET_END)) && (!is_param_expr())){ return expression(); }
            if (is_param_expr()){
                token = get_token();
            }
            return params();
            break;
        
        case NUM_TYPE:
        case STR_TYPE:
        case NULL_TYPE:
        case BOOL_TYPE:
            if (!match_token(BRACKET_END)){ return SYNTAX_ERROR; }
            return params();
            break;

        case COMMA:
            if (match_token(BRACKET_END)){ return SYNTAX_ERROR; }
            return params();
            break;

        case BRACKET_END:
            fprintf(out, "___________\n params OK return\n _____________\n");
            return OK;
            break;

        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;  
}

/**
 * @brief Function <BLOCK> on default calls command function
 * 
 * @return SYNTAX_ERROR or OK if finished
 */
int block(){
    fprintf(out, "nasli sme token v block: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BLOCK> -> <{> <NEWLINE> <COMMANDS> <}> <ELSE>*/
    switch(token.type){
        case BLOCK_START:
            if (!match_token(NEW_LINE)){ return SYNTAX_ERROR; }
            return block();
            break;

        case BLOCK_END:
            if (match_token(ELSE)){ 
                return cond_loop();
            } else if (token.type == NEW_LINE){
                fprintf(out, "___________\n block OK return \n _____________\n");
                return OK;
            } else { /* toto nie je iste */
                return SYNTAX_ERROR;
            }
            break;

        default:
            if (command() == OK){ return block(); }
            break;
        
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to call user made functions
 * 
 * @return Function return value (recursive), SYNTAX_ERROR for errors, OK if finished
 */
int func_call(){
    fprintf(out, "nasli sme token v func_call: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <FUNC_CALL> -> ID <(>  <PARAMS>  <)>*/
    switch(token.type){
        case ID:
            if (!match_token(BRACKET_START)){ return SYNTAX_ERROR; }
            return func_call();
            break;

        case BRACKET_START:
            if (match_token(BRACKET_END)){ 
                return func_call();
            } else {
                if (params() == OK){ return func_call(); }
            }
            break;

        case BRACKET_END:
            if (!match_token(NEW_LINE)){ return SYNTAX_ERROR; }
            fprintf(out, "___________\n func_call OK return \n _____________\n");
            return OK;
            break;

        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to call built in functions
 * 
 * @return OK or SYNTAX_ERROR
 */
int built_in_call(){
    fprintf(out, "nasli sme token v b_i_c: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BUILT_IN_CALL> -> <IFJ> <.> <KW> <(> <PARAMS> <)> */
    switch(token.type){
        case IFJ:
            if (!match_token(DOT)){ return SYNTAX_ERROR; }
            return built_in_call();
            break;
        
        case DOT: /* built in functions */
            if (!match_token(STRING)){ return SYNTAX_ERROR; }
            return built_in_call();
            break;

        /* built in functions KW ...*/

        case BRACKET_START:
            if (match_token(BRACKET_END)){ 
                return built_in_call();
            } else {
                if (params() == OK){ return built_in_call(); }
            }
            break;

        case BRACKET_END:
            if (!match_token(NEW_LINE)){ return SYNTAX_ERROR; }
            fprintf(out, "___________\n built_in_call OK return \n _____________\n");
            return OK;
            break;
        
        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to use conditions and while loops
 * @todo correctly check while {} else {} cannot happen
 * 
 * @return OK or SYNTAX_ERROR
 */
int cond_loop(){
    fprintf(out, "nasli sme token v cond_loop: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case IF:
            if (!match_token(BRACKET_START)){ return SYNTAX_ERROR; }
            return cond_loop();
            break;
        
        case WHILE:
            if (!match_token(BRACKET_START)){ return SYNTAX_ERROR; }
            return cond_loop();
            break;

        case BRACKET_START:
            if (match_token(BRACKET_END)){ 
                return SYNTAX_ERROR;
            } else { /* neskor bude treba zmenit na expression() == ok*/
                if (params() == OK) { return cond_loop(); }
            }
            break;

        case BRACKET_END:
            if (!match_token(BLOCK_START)){ return SYNTAX_ERROR; } 
            return cond_loop();
            break;
        
        case BLOCK_START:
            if ((block() == OK) && (token.type == ELSE)){ return cond_loop(); }
            if ((block() == OK) && (token.type == NEW_LINE)){ fprintf(out, "___________\n cond_loop OK return \n _____________\n"); return OK; }
            return SYNTAX_ERROR;
            break;

        case ELSE:
            if (!match_token(BLOCK_START)){
                return SYNTAX_ERROR;
            } else {
                if (block() == OK) { return OK; }
            }
            break;
        
        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to assign value or expression
 * 
 * @return OK or SYNTAX_ERROR
 */
int assign(){
    fprintf(out, "nasli sme token v assign: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <ASSIGN> -> <ID> <=> <LITERAL> (or) <EXPRESSION> */
    switch(token.type){

        case EQUAL:
            return assign();
            break;

        case ID:
        case STRING:
        case NUMBER:
        case GLOBAL_ID:
        case BOOLEAN:
            token = get_token();
            if (is_operator()){
                return expression();
            } else if (token.type == NEW_LINE) {
                fprintf(out, "___________\n assign OK return \n _____________\n");
                return OK;
            } else {
                return SYNTAX_ERROR;
            }
            break;

        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to declare user made functions
 * 
 * @return OK or SYNTAX_ERROR 
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
            } else {
                if (params() == OK){ return func_decl(); }
            }
            break;
        
        case BRACKET_END:
            if (!match_token(BLOCK_START)){ return SYNTAX_ERROR; }
            return func_decl();
            break;
        
        case BLOCK_START:
            if (block() == OK){ fprintf(out, "___________\n func_decl OK return \n _____________\n"); return OK; }
            break;
        
        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}


int command(){
    fprintf(out, "nasli sme token v command: ");
    print_token(token);
    fprintf(out, "\n");
   switch(token.type){
        case ID:
            if (match_token(BRACKET_START)){ 
                if (func_call() == OK){ return command(); }
            } else if (is_operator(token.type)) {
                if (expression() == OK){ return command(); } /* PSA */
            } else {
                fprintf(out, "ERROR command id else\n");
                return SYNTAX_ERROR;
            }
            break;

        case IFJ:
            if (built_in_call() == OK){ return command(); }
            break;

        case RETURN:
            print_token(token);
            fprintf(out, "\n");
            if ((!match_token(ID)) && (token.type != GLOBAL_ID) && (token.type != NUMBER) 
                && (token.type != STRING) && (token.type != BOOLEAN) && (token.type != NEW_LINE)){ 
                return SYNTAX_ERROR;
            } else {
                token = get_token(); /* pozor tu treba priradit hodnotu*/
                return command();
            }
            break;

        case GLOBAL_ID:
        case VAR:
            if (match_token(EQUAL)){ 
                if (assign() == OK){ return command(); }
            } else if (token.type == NEW_LINE){
                return command();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case IF:
            if (cond_loop() == OK){ return command(); }
            break;

        case STATIC:
            if (func_decl() == OK){ return command(); }
            break;
        
        case NEW_LINE:
            token = get_token();
            return command();
            break;

        case BLOCK_END:
            fprintf(out, "___________\n command OK return \n _____________\n");
            return OK; /* no other commands end block*/
            break;
        
        return SYNTAX_ERROR;
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
            if (match_token(NEW_LINE)){ return OK; }
            return SYNTAX_ERROR;
            break;
        
        case NEW_LINE:
            token = get_token();
            return valid();
            break;
        
        return SYNTAX_ERROR;
        
    }
    return SYNTAX_ERROR;
}

int program(){
    fprintf(out, "nasli sme token v prog: ");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case CLASS:
            if (match_token(ID)){ return program(); }
            return SYNTAX_ERROR;
            break;
        
        case ID:
            if (strcmp(token.value.string, "Program") != 0){
                return SYNTAX_ERROR;
            } else {
                if (!match_token(BLOCK_START)){ return SYNTAX_ERROR; }
                return program();
            }
            break;
        
        case BLOCK_START:
            fprintf(out, "___________\n built_in_call OK return \n _____________\n");
            return block();
            break;
        
        case NEW_LINE: /* nebezpecne opravit */
            token = get_token();
            return program();
            break;

        fprintf(out, "ERROR\n");
        return SYNTAX_ERROR;
    }
    return SYNTAX_ERROR;
}


int main (int argc, char** argv){
    (void)argc;
    (void)argv;

    in = fopen("../samples/ahoj.IFJcode25", "r");
    if (!in){ return SYNTAX_ERROR; }

    out = fopen("../samples/outfile.txt", "w");
    if (!out){ return SYNTAX_ERROR; }

    scanner_innit(in, out);

    bool ok;

    token = get_token();

    if (valid() == OK){
        fprintf(out, "VALID OK\n");
        ok = program();
    } else {
        fprintf(out, "VALID NOT OK\n");
    }


    fclose(in);
    fclose(out);

    return ok;
}
