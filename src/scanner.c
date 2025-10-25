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
char c;
char buffer[BUFFER_SIZE];
int i = 0; 

void reset_buffer() {
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
    return token;
}

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

    if (c == EOF) { return add_token(EOF_TOKEN); }

    // Whitespace
    while (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
        reset_buffer();
        c = advance();
    }

    //TODO ID and Keywords
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        while (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
        {
            c = advance();
        }
        ungetc(c, file);
        i--;
        buffer[i] = '\0';

        // Check if it's a keyword using lookup table
        TokenType keyword_type = lookup_keyword(buffer);
        return add_token(keyword_type);
    }

    //TODO number
    else if (c >= '0' && c <= '9') {
        while (c >= '0' && c <= '9') {
            c = peek();
            if (c >= '0' && c <= '9') {
                c = advance();
            } else {
                break;
            }
        }
        
        // Check for decimal point
        if (c == '.' && peek() >= '0' && peek() <= '9') {
            c = advance(); // consume the '.'
            while (c >= '0' && c <= '9') {
                c = peek();
                if (c >= '0' && c <= '9') {
                    c = advance();
                } else {
                    break;
                }
            }
        }
        
        return add_token(NUMBER);
    }

    else if (c == '/' && match('/')) {
        // Single line comment
        while (c != '\n' && c != EOF) {
            c = advance();
        }
        return get_token(); // Recursively get the next token
    }

    else if (c == '"') {
        // String literal
        c = advance();
        while (c != '"' && c != EOF) {
            c = advance();
        }
        if (c == '"') {
            advance(); // Consume closing quote
            return add_token(STRING);
        } else {
            return add_token(ERROR); // Unterminated string
        }
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
            return add_token(match('=') ? EQUAL_EQUAL : EQUAL); break;

        case '<':
            return add_token(match('=') ? LESS_EQUAL : LESS); break;

        case '>':
            return add_token(match('=') ? MORE_EQUAL : MORE); break;

        case '!':
            return add_token(match('=') ? NOT_EQUAL : NOT); break;

        case '&':
            return add_token(match('&') ? AND : ERROR); break;

        case '|':
            return add_token(match('|') ? OR : ERROR); break;

        // Special cases
        case '\n':
            return add_token(NEW_LINE); break;

        case EOF:
            return add_token(EOF_TOKEN); break;
            
        default:
            return add_token(ERROR); break;
    }
}

void print_token(Token token) {
    switch (token.type)
    {
    case ID:
        fprintf(output_file, "ID"); break;
    case GLOBAL_ID:
        fprintf(output_file, "GLOBAL_ID"); break;
    case CLASS:
        fprintf(output_file, "CLASS"); break;
    case IF:
        fprintf(output_file, "IF"); break;
    case ELSE:
        fprintf(output_file, "ELSE"); break;
    case IS:
        fprintf(output_file, "IS"); break;
    case NULL_KEYWORD:
        fprintf(output_file, "NULL_KEYWORD"); break;
    case RETURN:
        fprintf(output_file, "RETURN"); break;
    case VAR:
        fprintf(output_file, "VAR"); break;
    case WHILE:
        fprintf(output_file, "WHILE"); break;
    case IFJ:
        fprintf(output_file, "IFJ"); break;
    case STATIC:
        fprintf(output_file, "STATIC"); break;
    case IMPORT:
        fprintf(output_file, "IMPORT"); break;
    case FOR:
        fprintf(output_file, "FOR"); break;
    case NUM_TYPE:
        fprintf(output_file, "NUM_TYPE"); break;
    case STRING_TYPE:
        fprintf(output_file, "STRING_TYPE"); break;
    case NULL_TYPE:
        fprintf(output_file, "NULL_TYPE"); break;
    case NUMBER:
        fprintf(output_file, "NUMBER"); break;
    case STRING:
        fprintf(output_file, "STRING"); break;
    case BLOCK_START:
        fprintf(output_file, "BLOCK_START"); break;
    case BLOCK_END:
        fprintf(output_file, "BLOCK_END"); break;
    case BRACKET_START:
        fprintf(output_file, "BRACKET_START"); break;
    case BRACKET_END:
        fprintf(output_file, "BRACKET_END"); break;
    case DOT:
        fprintf(output_file, "DOT"); break;
    case COMMA:
        fprintf(output_file, "COMMA"); break;
    case PLUS:
        fprintf(output_file, "PLUS"); break;
    case MINUS:
        fprintf(output_file, "MINUS"); break;
    case MULTIPLY:
        fprintf(output_file, "MULTIPLY"); break;
    case DIVIDE:
        fprintf(output_file, "DIVIDE"); break;
    case EQUAL:
        fprintf(output_file, "EQUAL"); break;
    case EQUAL_EQUAL:
        fprintf(output_file, "EQUAL_EQUAL"); break;
    case LESS:
        fprintf(output_file, "LESS"); break;
    case LESS_EQUAL:
        fprintf(output_file, "LESS_EQUAL"); break;
    case MORE:
        fprintf(output_file, "MORE"); break;
    case MORE_EQUAL:
        fprintf(output_file, "MORE_EQUAL"); break;
    case NOT:
        fprintf(output_file, "NOT"); break;
    case NOT_EQUAL:
        fprintf(output_file, "NOT_EQUAL"); break;
    case AND:
        fprintf(output_file, "AND"); break;
    case OR:
        fprintf(output_file, "OR"); break;
    case NEW_LINE:
        fprintf(output_file, "NEW_LINE"); break;
    case EOF_TOKEN:
        fprintf(output_file, "EOF"); break;
    case ERROR:
        fprintf(output_file, "ERROR"); break;
    default:
        fprintf(output_file, "UNKNOWN"); break;
    }

    fprintf(output_file, " [");
    for (int j = 0; j < i; ++j) {
        fputc(buffer[j], output_file);
    }
    fprintf(output_file, "]\n");
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
    if (!file) {return 1;}

    output_file = fopen("tokens.txt", "w");
    if (!output_file) {return 1;}

    prototype_parser_function();

    fclose(file);
    fclose(output_file);

    return 0;
}

