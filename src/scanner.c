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

FILE *file;                 // Input file from stdin
FILE *out;                  // Output file to build/tokens.txt
char c;                     // Current character
char buffer[BUFFER_SIZE];   // Current token buffer
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
        reset_buffer();
        c = advance();
        if (c == '"') {
            c = advance();
            if (c == '"') { // multiline string start
                reset_buffer();
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
                i -= 3;
                return add_token(STRING); // multiline string
            } else {
                reset_buffer();
                return add_token(STRING); // empty string
            }
        }
        else {
            while (c != '"' && c != EOF && c != '\n') {
                c = advance();
            }
            if (c == '"') {
                i -= 1;
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
        case ID:            fprintf(out, "ID"); break;
        case GLOBAL_ID:     fprintf(out, "GLOBAL_ID"); break;
        case CLASS:         fprintf(out, "CLASS"); break;
        case IF:            fprintf(out, "IF"); break;
        case ELSE:          fprintf(out, "ELSE"); break;
        case IS:            fprintf(out, "IS"); break;
        case NULL_KEYWORD:  fprintf(out, "NULL_KEYWORD"); break;
        case RETURN:        fprintf(out, "RETURN"); break;
        case VAR:           fprintf(out, "VAR"); break;
        case WHILE:         fprintf(out, "WHILE"); break;
        case IFJ:           fprintf(out, "IFJ"); break;
        case STATIC:        fprintf(out, "STATIC"); break;
        case IMPORT:        fprintf(out, "IMPORT"); break;
        case FOR:           fprintf(out, "FOR"); break;
        case NUM_TYPE:      fprintf(out, "NUM_TYPE"); break;
        case STR_TYPE:      fprintf(out, "STR_TYPE"); break;
        case NULL_TYPE:     fprintf(out, "NULL_TYPE"); break;
        case BOOL_TYPE:     fprintf(out, "BOOL_TYPE"); break;
        case INTEGER:       fprintf(out, "INTEGER"); break;
        case FLOATING:      fprintf(out, "FLOATING"); break;
        case STRING:        fprintf(out, "STRING"); break;
        case BOOLEAN:       fprintf(out, "BOOLEAN"); break;
        case BLOCK_START:   fprintf(out, "BLOCK_START"); break;
        case BLOCK_END:     fprintf(out, "BLOCK_END"); break;
        case BRACKET_START: fprintf(out, "BRACKET_START"); break;
        case BRACKET_END:   fprintf(out, "BRACKET_END"); break;
        case COLON:         fprintf(out, "COLON"); break;
        case QUESTION:      fprintf(out, "QUESTION"); break;
        case DOT:           fprintf(out, "DOT"); break;
        case COMMA:         fprintf(out, "COMMA"); break;
        case PLUS:          fprintf(out, "PLUS"); break;
        case MINUS:         fprintf(out, "MINUS"); break;
        case MULTIPLY:      fprintf(out, "MULTIPLY"); break;
        case DIVIDE:        fprintf(out, "DIVIDE"); break;
        case EQUAL:         fprintf(out, "EQUAL"); break;
        case EQUAL_EQUAL:   fprintf(out, "EQUAL_EQUAL"); break;
        case LESS:          fprintf(out, "LESS"); break;
        case LESS_EQUAL:    fprintf(out, "LESS_EQUAL"); break;
        case MORE:          fprintf(out, "MORE"); break;
        case MORE_EQUAL:    fprintf(out, "MORE_EQUAL"); break;
        case NOT:           fprintf(out, "NOT"); break;
        case NOT_EQUAL:     fprintf(out, "NOT_EQUAL"); break;
        case AND:           fprintf(out, "AND"); break;
        case OR:            fprintf(out, "OR"); break;
        case NEW_LINE:      fprintf(out, "NEW_LINE"); break;
        case EOF_TOKEN:     fprintf(out, "EOF"); break;
        case ERROR:         fprintf(out, "ERROR"); break;
        default:            fprintf(out, "UNKNOWN"); break;
    }

    if (token.type == INTEGER) {
        fprintf(out, "        INT[%d]", token.value.integer);
    } else if (token.type == STRING) {
        fprintf(out, "         STR[%s]", token.value.string);
    } else if (token.type == ID) {
        fprintf(out, "             STR[%s]", token.value.string);
    } else if (token.type == GLOBAL_ID) {
        fprintf(out, "      STR[%s]", token.value.string);
    }

    fprintf(out, "\n");
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

    file = stdin;

    out = fopen("../build/tokens.txt", "w");
    if (!out) {return 1;}

    prototype_parser_function();

    fclose(out);

    return 0;
}

