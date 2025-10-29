/**
 * @file scanner.c
 * @author Petr David Lanca
 * @brief Scanner implementation for tokenizing input
 * @date 2025-10-01
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

FILE *file;
FILE *output_file;

char c;                     // Current character
char buffer[BUFFER_SIZE];
int i = 0;                  // Buffer index

void reset_buffer() {
    memset(buffer, 0, BUFFER_SIZE);
    i = 0;
}

char advance() {
    c = fgetc(file);
    buffer[i] = c;
    i++;
    return c;
}

char peek() {
    c = fgetc(file);
    ungetc(c, file);
    return c;
}

bool match(char d) {
    c = peek();
    return (c == d);
}

Token add_token(TokenType type) {
    Token token;
    token.type = type;

    if (token.type == STRING || token.type == ID || token.type == GLOBAL_ID) {
        strncpy(token.value.string, buffer, i);
        token.value.string[i] = '\0';
    }

    if (token.type == INTEGER) {
        buffer[i] = '\0';
        token.value.integer = atoi(buffer);
    }

    return token;
}


// === Keyword lookup table ========================================
typedef struct {
    char* keyword;
    TokenType token_type;
} KeywordEntry;

static KeywordEntry keyword_table[] = {
    {"class", CLASS},
    {"if", IF},
    {"else", ELSE},
    {"is", IS},
    {"null", NULL_KEYWORD},
    {"return", RETURN},
    {"var", VAR},
    {"while", WHILE},
    {"Ifj", IFJ},
    {"static", STATIC},
    {"import", IMPORT},
    {"for", FOR},
    {"Num", NUM_TYPE},
    {"String", STR_TYPE},
    {"Null", NULL_TYPE},
    {"Bool", BOOL_TYPE},
    {NULL, 0}               // Terminator
};

TokenType lookup_keyword(const char* word) {
    for (int j = 0; keyword_table[j].keyword != NULL; j++) {
        if (strcmp(word, keyword_table[j].keyword) == 0) {
            return keyword_table[j].token_type;
        }
    }
    return ID;
}

// === Where magic happens ========================================
Token get_token() {

    // Reset buffer and read first character
    reset_buffer();
    c = advance();

    // Ignore whitespace
    while (c == ' ' || c == '\t') {
        reset_buffer();
        c = advance();
    }

    // End of file
    if (c == EOF) { return add_token(EOF_TOKEN); }

    // Comments, division operator
    if (c == '/') {

        // Single-line comment
        if (match('/')) {
            advance();
            while (c != '\n' && c != EOF) {
                c = advance();
            }
            return get_token();
        }

        // Multi-line comment
        else if (match('*')) {
            advance();
            c = advance();
            while (true) {
                if (c == EOF) {
                    return add_token(ERROR);
                }
                if (c == '*' && match('/')) {
                    advance();
                    break;
                }
                c = advance();
            }
            return get_token();
        }

        // Division operator
        else {
            return add_token(DIVIDE);
        }
    }

    // TODO Keywords and ID
    else if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        char p = peek();
        while (p >= 'a' && p <= 'z' || p >= 'A' && p <= 'Z' || p >= '0' && p <= '9' || p == '_') {
            c = advance();
            p = peek();
        }

        // Keyword check
        return add_token(lookup_keyword(buffer));
    }

    // Global ID
    else if (c == '_') {
        if (match('_')) {
            c = advance();
            char p = peek();
            while (p >= 'a' && p <= 'z' || p >= 'A' && p <= 'Z' || p >= '0' && p <= '9' || p == '_') {
                c = advance();
                p = peek();
            }
            return add_token(GLOBAL_ID);
        }
        else {
            return add_token(ERROR);
        }
    }

    // Integers, TODO float, hex, exponents
    else if (c >= '1' && c <= '9') {
        while (peek() >= '0' && peek() <= '9') {
            c = advance();
        }
        
        return add_token(INTEGER);
    }

    // Strings, TODO Multiline strings
    else if (c == '"') {
        c = advance();
        if (c == '"') {
            c = advance();
            if (c == '"') { // multiline string start
                int counter = 0;
                while (true) 
                {
                    c = advance();
                    if (c == '"') {
                        counter++;
                        if (counter == 3) {
                            break;
                        }
                    }
                    else {
                        counter = 0;
                    }
                }
                return add_token(STRING); // multiline string
            } else {
                return add_token(STRING); // empty string
            }
        }
        else {
            while (c != '"' && c != EOF && c != '\n') {
                c = advance();
            }
            if (c == '"') {
                return add_token(STRING);
            } else {
                return add_token(ERROR);
            }
        }
    }

    else switch (c) {
        case '{': return add_token(BLOCK_START);                        // One character tokens
        case '}': return add_token(BLOCK_END);
        case '(': return add_token(BRACKET_START);
        case ')': return add_token(BRACKET_END);
        case '.': return add_token(DOT);
        case ',': return add_token(COMMA);
        case '+': return add_token(PLUS);
        case '-': return add_token(MINUS);
        case '*': return add_token(MULTIPLY);
        case ':': return add_token(COLON);
        case '?': return add_token(QUESTION);
        case '=': return add_token(match('=') ? EQUAL_EQUAL : EQUAL);   // Two character tokens
        case '<': return add_token(match('=') ? LESS_EQUAL : LESS);
        case '>': return add_token(match('=') ? MORE_EQUAL : MORE);
        case '!': return add_token(match('=') ? NOT_EQUAL : NOT);
        case '&': return add_token(match('&') ? AND : ERROR);
        case '|': return add_token(match('|') ? OR : ERROR);
        case '\n': return add_token(NEW_LINE);                          // Special cases
        default: return add_token(ERROR);
    }
}

void print_token(Token token) {
    switch (token.type){
        case ID:            fprintf(output_file, "ID"); break;
        case GLOBAL_ID:     fprintf(output_file, "GLOBAL_ID"); break;
        case CLASS:         fprintf(output_file, "CLASS"); break;
        case IF:            fprintf(output_file, "IF"); break;
        case ELSE:          fprintf(output_file, "ELSE"); break;
        case IS:            fprintf(output_file, "IS"); break;
        case NULL_KEYWORD:  fprintf(output_file, "NULL_KEYWORD"); break;
        case RETURN:        fprintf(output_file, "RETURN"); break;
        case VAR:           fprintf(output_file, "VAR"); break;
        case WHILE:         fprintf(output_file, "WHILE"); break;
        case IFJ:           fprintf(output_file, "IFJ"); break;
        case STATIC:        fprintf(output_file, "STATIC"); break;
        case IMPORT:        fprintf(output_file, "IMPORT"); break;
        case FOR:           fprintf(output_file, "FOR"); break;
        case NUM_TYPE:      fprintf(output_file, "NUM_TYPE"); break;
        case STR_TYPE:      fprintf(output_file, "STR_TYPE"); break;
        case NULL_TYPE:     fprintf(output_file, "NULL_TYPE"); break;
        case BOOL_TYPE:     fprintf(output_file, "BOOL_TYPE"); break;
        case INTEGER:       fprintf(output_file, "INTEGER"); break;
        case FLOATING:      fprintf(output_file, "FLOATING"); break;
        case STRING:        fprintf(output_file, "STRING"); break;
        case BOOLEAN:       fprintf(output_file, "BOOLEAN"); break;
        case BLOCK_START:   fprintf(output_file, "BLOCK_START"); break;
        case BLOCK_END:     fprintf(output_file, "BLOCK_END"); break;
        case BRACKET_START: fprintf(output_file, "BRACKET_START"); break;
        case BRACKET_END:   fprintf(output_file, "BRACKET_END"); break;
        case COLON:         fprintf(output_file, "COLON"); break;
        case QUESTION:      fprintf(output_file, "QUESTION"); break;
        case DOT:           fprintf(output_file, "DOT"); break;
        case COMMA:         fprintf(output_file, "COMMA"); break;
        case PLUS:          fprintf(output_file, "PLUS"); break;
        case MINUS:         fprintf(output_file, "MINUS"); break;
        case MULTIPLY:      fprintf(output_file, "MULTIPLY"); break;
        case DIVIDE:        fprintf(output_file, "DIVIDE"); break;
        case EQUAL:         fprintf(output_file, "EQUAL"); break;
        case EQUAL_EQUAL:   fprintf(output_file, "EQUAL_EQUAL"); break;
        case LESS:          fprintf(output_file, "LESS"); break;
        case LESS_EQUAL:    fprintf(output_file, "LESS_EQUAL"); break;
        case MORE:          fprintf(output_file, "MORE"); break;
        case MORE_EQUAL:    fprintf(output_file, "MORE_EQUAL"); break;
        case NOT:           fprintf(output_file, "NOT"); break;
        case NOT_EQUAL:     fprintf(output_file, "NOT_EQUAL"); break;
        case AND:           fprintf(output_file, "AND"); break;
        case OR:            fprintf(output_file, "OR"); break;
        case NEW_LINE:      fprintf(output_file, "NEW_LINE"); break;
        case EOF_TOKEN:     fprintf(output_file, "EOF"); break;
        case ERROR:         fprintf(output_file, "ERROR"); break;
        default:            fprintf(output_file, "UNKNOWN"); break;
    }

    if (token.type == INTEGER) {
        fprintf(output_file, "        INT[%d]", token.value.integer);
    } else if (token.type == STRING) {
        fprintf(output_file, "         STR[%s]", token.value.string);
    } else if (token.type == ID) {
        fprintf(output_file, "             STR[%s]", token.value.string);
    } else if (token.type == GLOBAL_ID) {
        fprintf(output_file, "      STR[%s]", token.value.string);
    }

    fprintf(output_file, "\n");
}

void prototype_parser_function() {
    Token token;
    do {
        token = get_token();
        print_token(token);
    } while (token.type != EOF_TOKEN);
}

int main(int argc, char const *argv[])
{
    file = fopen("../samples/ex4-multiline-strings.wren", "r");
    if (!file) {return 1;}

    output_file = fopen("../build/tokens.txt", "w");
    if (!output_file) {return 1;}

    prototype_parser_function();

    fclose(file);
    fclose(output_file);

    return 0;
}

