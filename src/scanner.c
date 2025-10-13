#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

FILE *file;
char c;
char buffer[BUFFER_SIZE];
char i = 0;
char keywords[KEYWORD_LIST_LENGTH][255] = {
    "class",
    "if",
    "else",
    "is",
    "null",
    "return",
    "var",
    "while",
    "Ifj",
    "static",
    "import",
    "for",
    "Num",
    "String",
    "Null"
};

void reset_buffer() {
    i = 0;
    for (int j = 0; j < BUFFER_SIZE; j++) {
        buffer[j] = 0;
    }
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
    return token;
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

        for (int j = 0; j < KEYWORD_LIST_LENGTH; j++)
        {
            if (strcmp(buffer, keywords[j])) {
                return add_token(KEYWORD);
            }
        }

        return add_token(ID);
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
            return add_token(match('&') ? AND : EOF_TOKEN);

        case '|':
            return add_token(match('|') ? OR : EOF_TOKEN);

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

