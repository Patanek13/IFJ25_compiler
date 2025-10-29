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

bool is_operator(){
    return ((token.type == PLUS) || (token.type == MINUS) || (token.type == MULTIPLY)
            || (token.type == DIVIDE));
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
                return OK;
            } else { /* toto nie je iste */
                return SYNTAX_ERROR;
            }
            break;

        default:
            if (command() == OK){ return block(); }
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
                if (params() == OK){ return func_call(); }
            }
            break;

        case BRACKET_END:
            if (!match(NEW_LINE)){ return SYNTAX_ERROR; }
            return OK;
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
                if (params() == OK){ return built_in_call(); }
            }
            break;

        case BRACKET_END:
            if (!match(NEW_LINE)){ return SYNTAX_ERROR; }
            return OK;
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
                if (params() == OK) { return cond_loop(); }
            }
            break;

        case BRACKET_END:
            if (!match(BLOCK_START)){
                return SYNTAX_ERROR;
            } else {
                if (block() == OK) { return cond_loop(); }
            }
            break;

        case ELSE:
            if (!match(BLOCK_START)){
                return SYNTAX_ERROR;
            } else {
                if (block() == OK) { return OK; }
            }
            break;
        return SYNTAX_ERROR;
    }
}

int assign(){
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
                return OK;
            } else {
                return SYNTAX_ERROR;
            }
            break;

        return SYNTAX_ERROR;
    }
}

int func_decl(){ /* <func_decl> -> STATIC ID <=?> <brackets> <block>*/
    switch(token.type){
        case STATIC:
            if (!match(ID)){ return SYNTAX_ERROR; }
            return func_decl();
            break;
        
        case ID:
            if (match(BRACKET_START)){ /* user made */
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
            if (!match(BRACKET_START)){ return SYNTAX_ERROR; }
            return func_decl();
            break;
        
        case BRACKET_START:
            if (match(BRACKET_END)){ 
                return func_decl();
            } else {
                if (params() == OK){ return func_decl(); }
            }
            break;
        
        case BRACKET_END:
            if (!match(BLOCK_START)){ return SYNTAX_ERROR; }
            return func_decl();
            break;
        
        case BLOCK_START:
            if (block() == OK){ return OK; }
            break;
        
        return SYNTAX_ERROR;
    }
}


int command(){
   switch(token.type){
        case ID:
            if (match(BRACKET_START)){ 
                if (func_call() == OK){ return command(); }
            } else if (is_operator(token.type)) {
                if (expression() == OK){ return command(); } /* PSA */
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case IFJ:
            if (built_in_call() == OK){ return command(); }
            break;

        case RETURN:
            if ((!match(ID)) && (token.type != GLOBAL_ID) && (token.type != NUMBER) 
                && (token.type != STRING) && (token.type != BOOLEAN)){ 
                return SYNTAX_ERROR;
            } else if(!match(NEW_LINE)) {
                return SYNTAX_ERROR; 
            } else {
                return command();
            }
            break;

        case GLOBAL_ID:
        case VAR:
            if (match(EQUAL)){ 
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
            return OK; /* no other commands end block*/
            break;
        
        return SYNTAX_ERROR;
   }
}
