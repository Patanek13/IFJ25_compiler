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
char c;
char buffer[BUFFER_SIZE];
int index = 0; 

void reset_buffer() {
    index = 0;
    for (int j = 0; j < BUFFER_SIZE; j++) {
        buffer[j] = 0;
    }
}

char advance() {
    c = fgetc(file);
    buffer[index] = c;
    index++;
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
    return token;
}

typedef struct {
    char* keyword;
    TokenType token_type;
} KeywordEntry;

// Static keyword lookup table
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
    {"String", STRING_TYPE},
    {"Null", NULL_TYPE},
    {NULL, 0} // Sentinel
};

TokenType lookup_keyword(const char* word) {
    for (int i = 0; keyword_table[i].keyword != NULL; i++) {
        if (strcmp(word, keyword_table[i].keyword) == 0) {
            return keyword_table[i].token_type;
        }
    }
    return ID; // Not a keyword, return identifier
}

Token get_token() {
    Token token;

    // Reset buffer
    reset_buffer();

    c = advance();

    // Whitespace
    while (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
        reset_buffer();
        c = advance();
    }

    //TODO ID and Keywords
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        while (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
        {
            c = peek();
            if (c == ' ' || c == '\n') {
                break;
            }
            c = advance();
        }

        // Check if it's a keyword using lookup table
        TokenType keyword_type = lookup_keyword(buffer);
        return add_token(keyword_type);
    }

    //TODO number
    else if (c >= '0' && c <= '9') {
        return add_token(NUMBER);
    }

    else switch (c)
    {
        // One character tokens
        case '{':
            return add_token(BLOCK_START); break;

        case '}':
            return add_token(BLOCK_END); break;

        case '(':
            return add_token(BRACKET_START); break;

        case ')':
            return add_token(BRACKET_END); break;

        case '.':
            return add_token(DOT); break;

        case '+':
            return add_token(PLUS); break;

        case '-':
            return add_token(MINUS); break;

        case '*':
            return add_token(MULTIPLY); break;

        case '/':
            return add_token(DIVIDE); break;

        // Two character tokens
        case '=':
            return add_token(match('=') ? EQUAL_EQUAL : EQUAL);

        case '<':
            return add_token(match('=') ? LESS_EQUAL : LESS);

        case '>':
            return add_token(match('=') ? MORE_EQUAL : MORE);

        case '!':
            return add_token(match('=') ? NOT_EQUAL : NOT);

        case '&':
            return add_token(match('&') ? AND : ERROR);

        case '|':
            return add_token(match('|') ? OR : ERROR);

        case '\n':
            return add_token(NEW_LINE); break;

        case EOF:
            return add_token(EOF_TOKEN); break;
            
        default:
            printf("Lexical ERROR\n");
            break;
    }
}

void print_token(Token token) {
    switch (token.type)
    {
    case ID:
        printf("ID"); break;

    case EQUAL:
        printf("EQUAL"); break;

    case NUMBER:
        printf("NUMBER"); break;

    case EOF_TOKEN:
        printf("EOF"); break;
    
    default:
        printf("UNKNOWN"); break;
        break;
    }

    printf(" [%s]\n", buffer);
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
    file = fopen("../samples/ahoj.IFJcode25", "r");
    if (!file) {
        fprintf(stderr, "Error: cannot open file\n");
        return 1;
    }

    prototype_parser_function();

    fclose(file);

    return 0;
}

