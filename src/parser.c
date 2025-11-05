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

Token token;
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

bool is_operator(){
    return ((token.type == PLUS) || (token.type == MINUS) || (token.type == MULTIPLY) || (token.type == DIVIDE));
}

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

int ternary(){ /*cela kontrola tokenov je neskutocne chaba mozno bude treba novy state brackets idk toto je strasna picovina*/
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
        case VAR:
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
            fprintf(out, "___________\n block OK return \n_____________\n");
            token = get_token();
            return ERR_OK;
            break;

        default:
            if (command() == ERR_OK){ return block(); }
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
            if (params() == ERR_OK){ return func_call(); }
            return SYNTAX_ERROR;
            break;

        case BRACKET_END:
            // if (!match_token(NEW_LINE)){ return SYNTAX_ERROR; }
            // token = get_token();
            fprintf(out, "___________\n func_call OK return \n_____________\n");
            return ERR_OK;
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to call built in functions
 * 
 * @return ERR_OK or SYNTAX_ERROR
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
        
        case DOT:
            if (!match_token(ID)){ print_token(token); return SYNTAX_ERROR; }
            return built_in_call();
            break;

        case ID:
            if (is_built_in_func()){
                token = get_token();
                print_token(token);
                return built_in_call();
            }
            return SYNTAX_ERROR;
            break;
    

        case BRACKET_START:
            if (params() == ERR_OK){ return built_in_call(); }
            return SYNTAX_ERROR;
            break;

        case BRACKET_END:
            // if (!match_token(NEW_LINE)){ return SYNTAX_ERROR; }
            // token = get_token();
            fprintf(out, "___________\n built_in_call OK return \n_____________\n");
            return ERR_OK;
            break;
        
        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

/**
 * @brief Function to use conditions and while loops
 * @todo doplnit kde mozu byt newlines
 * 
 * @return ERR_OK or SYNTAX_ERROR
 */
int cond_loop(){
    fprintf(out, "nasli sme token v cond_loop cond_state: %s: ", (cond_state)? "while":"if");
    print_token(token);
    fprintf(out, "\n");
    switch(token.type){
        case IF:
            if (!match_token(BRACKET_START)){ return SYNTAX_ERROR; }
            cond_state = 0;
            return cond_loop();
            break;
        
        case WHILE:
            if (!match_token(BRACKET_START)){ fprintf(out, "failed while"); return SYNTAX_ERROR; }
            cond_state = 1;
            return cond_loop();
            break;

        case BRACKET_START:
            if (match_token(BRACKET_END)){ 
                return SYNTAX_ERROR;
            } else { /* neskor bude treba zmenit na expression() == ERR_OK*/
                if (params() == ERR_OK) { return cond_loop(); }
            }
            break;

        case BRACKET_END:
            if (!match_token(BLOCK_START)){ fprintf(out, "next token: "); print_token(token); return SYNTAX_ERROR; } 
            return cond_loop();
            break;
        
        case BLOCK_START:
            if (!cond_state){
                if (block() == ERR_OK){
                    if (token.type == ELSE){ return cond_loop(); }
                    else if (token.type == NEW_LINE){ fprintf(out, "___________\n if OK return 1\n_____________\n"); return ERR_OK; }
                }                
            } else {
                if ((block() == ERR_OK) && (token.type == NEW_LINE)){ fprintf(out, "___________\n while OK return \n_____________\n"); return ERR_OK; }
            }
            return SYNTAX_ERROR;
            break;

        case ELSE:
            if (match_token(IF)){
                return cond_loop();
            } else if (token.type == BLOCK_START){
                fprintf(out, "starting else recursion\n");
                if ((block() == ERR_OK) && (token.type == NEW_LINE)){ fprintf(out, "___________\n else OK return \n_____________\n"); return ERR_OK; }
            }
            fprintf(out, "SYNTAX ERROR ELSE");
            return SYNTAX_ERROR;
            break;
        
        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
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
        case VAR:
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

        case GLOBAL_ID:
        case VAR:
            if (match_token(EQUAL)){ 
                if (assign() == ERR_OK){ return command(); }
            } else if (token.type == NEW_LINE){
                return command();
            } else {
                fprintf(out, "globalid or var next token wasnt newline or =\n");
                return SYNTAX_ERROR;
            }
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
            fprintf(out, "___________\n built_in_call OK return \n_____________\n");
            return block();
            break;
        
        case NEW_LINE: /* nebezpecne opravit */
            if (!match_token(CLASS)){ return SYNTAX_ERROR; }
            return program();
            break;
        
        default:
            fprintf(out, "ERROR\n");
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}


int main (int argc, char** argv){
    (void)argc;
    (void)argv;

    in = fopen("../samples/ahoj.IFJcode25", "r");
    if (!in){ printf("nejde otvorit\n"); return SYNTAX_ERROR; }

    out = fopen("../samples/outfile.txt", "w");
    if (!out){ return SYNTAX_ERROR; }

    scanner_init(in, out);

    int ok;

    token = get_token();

    if (valid() == ERR_OK){
        fprintf(out, "VALID OK\n");
        if (program() == ERR_OK){
            ok = 1;
        } else {
            ok = 0;
        }
    } else {
        fprintf(out, "VALID NOT OK\n");
    }

    fprintf(out, "Program ended %s\n", (ok == 1) ? "successfully" : "with error");
    fclose(in);
    fclose(out);

    
    return ok;
}
