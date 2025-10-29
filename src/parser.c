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

FILE* src;
FILE* out;

Token token;

bool match(TokenType type){
    token = get_token();
    return (token.type == type);
}

int block(){
    switch(token.type){
        case BLOCK_START:
            if (!match(NEW_LINE)){ return SYNTAX_ERROR; }
            return block();
            break;

        case BLOCK_END:
            if (match(ELSE)){ 
                return cond_loop();
            } else if (token.type == NEW_LINE){
                return ERR_OK;
            }
            break;

        default:
            return command();
            break;
        
    }
}

int func_call(){
    switch(token.type){
        case ID:
            if (!match(BRACKET_START)){ return SYNTAX_ERROR; }
            return func_call();
            break;

        case BRACKET_START:
            if (match(BRACKET_END)){ 
                return func_call();
            } else {
                return params();
            }
            break;

        case BRACKET_END:
            if (!match(NEW_LINE)){ return SYNTAX_ERROR; }
            return ERR_OK;
            break;

        return SYNTAX_ERROR;
    }
}

int built_in_call(){
    switch(token.type){
        case IFJ:
            if (!match(DOT)){ return SYNTAX_ERROR; }
            return built_in_call();
            break;
        
        case DOT: /* built in functions */
            if (!match(STRING)){ return SYNTAX_ERROR; }
            return built_in_call();
            break;

        /* built in functions KW ...*/

        case BRACKET_START:
            if (match(BRACKET_END)){ 
                return built_in_call();
            } else {
                return params();
            }
            break;

        case BRACKET_END:
            if (!match(NEW_LINE)){ return SYNTAX_ERROR; }
            return ERR_OK;
            break;
        
        return SYNTAX_ERROR;
    }
}

int cond_loop(){
    switch(token.type){
        case IF:
            if (!match(BRACKET_START)){ return SYNTAX_ERROR; }
            return cond_loop;
            break;

        case BRACKET_START:
            if (match(BRACKET_END)){ 
                return SYNTAX_ERROR;
            } else {
                return params();
            }
            break;

        case BRACKET_END:
            if (!match(BLOCK_START)){
                return SYNTAX_ERROR;
            } else {
                return block();
            }
            break;

        case ELSE:
            if (!match(BLOCK_START)){
                return SYNTAX_ERROR;
            } else {
                return block();
            }
            break;
        return SYNTAX_ERROR;
    }
}


int command(){
   switch(token.type){
        case ID:
            if (match(BRACKET_START)){ 
                return func_call();
            } else if (is_operator(token.type)) {
                return expression(); /* PSA */
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case IFJ:
            return built_in_call();
            break;

        case RETURN:
            token = get_token();
            if ((token.type != ID) && (token.type != GLOBAL_ID) && (token.type != NUMBER) 
                && (token.type != STRING) && (token.type != BOOLEAN)){ return SYNTAX_ERROR; }
            break;

        case GLOBAL_ID:
        case VAR:
            if (match(EQUAL)){ 
                return assign();
            } else if (token.type == NEW_LINE){
                return command();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case IF:
            return cond_loop();
            break;

        case STATIC:
            return func_decl();
            break;

        case BLOCK_END:
            return ERR_OK; /* no other commands end block*/
            break;
        
        return SYNTAX_ERROR;
   }
}
