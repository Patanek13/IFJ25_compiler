/**
 * @file scanner.c
 * @author Petr David Lanca
 * @brief Scanner for tokenizing input
 * @date 2.11.2025
 *
 */

// RUN TESTS
// make && ../build/IFJcompiler -s < ../lex_tests/test1.wren > ../lex_tests/test_output.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "scanner.h"
#include "error.h"

//------------------------------------- Global variables -----------------------------------------

FILE *file;                         // Input file (stdin)
FILE *output;                       // Output file (stdout)
char c;                             // Current character
char buffer[BUFFER_SIZE];           // Current token buffer
int i = 0;                          // Buffer index
bool was_new_line = false;          // If last generated token was NEW_LINE

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
    {"true", BOOLEAN},
    {"false", BOOLEAN},
    {NULL, 0}
};

//================================================================================================
//                                      BASIC FUNCTIONS
//================================================================================================

// Return textual representation for single-character operators used when
// creating OPERATOR tokens.
static const char *op_string(TokenType type) {
    switch (type) {
        case PLUS: return "+";
        case MINUS: return "-";
        case MULTIPLY: return "*";
        case DIVIDE: return "/";
        default: return "";
    }
}

void scanner_init(FILE *input, FILE *output_file) {
    file = input;
    output = output_file;
}

//------------------------------------- Buffer ---------------------------------------------------

void reset_buffer() {
    memset(buffer, 0, BUFFER_SIZE);
    i = 0;
}

//------------------------------------- Character scanning ---------------------------------------

char advance() {
    c = fgetc(file);
    if (i < BUFFER_SIZE - 1) {
        buffer[i] = c;
        i++;
    }
    return c;
}

char peek() {
    char next_char = fgetc(file);
    ungetc(next_char, file);
    return next_char;
}

bool match(char expected) {
    char next_char = peek();
    if (next_char == expected) {
        advance();
        return true;
    }
    return false;
}

void replace(char ch) {
    buffer[i-1] = ch;
}

//------------------------------------- Token creation -------------------------------------------

Token add_token(TokenType type) {
    Token token;
    token.type = type;

    switch (type) {
        case ID:
        case GLOBAL_ID:
        case STRING:
            buffer[i] = '\0';
            strncpy(token.value.string, buffer, BUFFER_SIZE - 1);
            token.value.string[BUFFER_SIZE - 1] = '\0';
            break;

        case PLUS:
        case MINUS:
        case MULTIPLY:
        case DIVIDE:
            strncpy(token.value.string, op_string(type), BUFFER_SIZE - 1);
            token.value.string[BUFFER_SIZE - 1] = '\0';
            token.type = OPERATOR;
            break;

        case INTEGER:
            buffer[i] = '\0';
            token.value.integer = strtol(buffer, NULL, 0);
            break;

        case FLOATING:
            buffer[i] = '\0';
            token.value.floating = atof(buffer);
            break;

        case BOOLEAN:
            buffer[i] = '\0';
            if (strcmp(buffer, "true") == 0) {
                token.value.boolean = true;
            } else {
                token.value.boolean = false;
            }
            break;

        default:
            break;
    }

    return token;
}

//------------------------------------- Keyword lookup table -------------------------------------

TokenType lookup_keyword(const char* word) {
    for (int j = 0; keyword_table[j].keyword != NULL; j++) {
        if (strcmp(word, keyword_table[j].keyword) == 0) {
            return keyword_table[j].type;
        }
    }
    return ID;
}

//================================================================================================
//                                      SCANNER STATES
//================================================================================================

//------------------------------------- Comments -------------------------------------------------

Token single_line_comment() {
    advance();
    while (c != '\n' && c != EOF) {
        advance();
    }
    ungetc(c, file);
    return get_token();
}

Token multi_line_comment() {
    int depth = 1;
    advance();

    while (depth > 0) {
        if (c == EOF) {
            return add_token(ERROR);
        }

        if (c == '/' && peek() == '*') {
            advance();
            depth++;
        }
        else if (c == '*' && peek() == '/') {
            advance();
            depth--;
            if (depth == 0) {
                advance();
                break;
            }
        }

        advance();
    }

    ungetc(c, file);
    return get_token();
}

Token scan_slash() {
    if (match('/')) {
        return single_line_comment();
    }

    else if (match('*')) {
        return multi_line_comment();
    }

    else {
        return add_token(DIVIDE);
    }
}

//------------------------------------- IDs and keywords -----------------------------------------

Token scan_identifier() {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }
    buffer[i] = '\0';
    return add_token(lookup_keyword(buffer));
}

Token scan_global_id() {
    advance();
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }
    buffer[i] = '\0';
    return add_token(GLOBAL_ID);
}

//------------------------------------- Numbers --------------------------------------------------

Token scan_exponent() {
    if (match('+') || match('-')) {
        advance();
    }

    if (!isdigit(peek())) {
        return add_token(ERROR);
    }

    while (isdigit(peek())) {
        advance();
    }

    return add_token(FLOATING);
}

Token scan_floating() {
    if (!isdigit(peek())) {
        return add_token(ERROR);
    }

    while (isdigit(peek())) {
        advance();
    }

    if (match('e') || match('E')) {
        return scan_exponent();
    }

    return add_token(FLOATING);
}

Token scan_number() {
    while (isdigit(peek())) {
        advance();
    }

    if (match('.')) {
        return scan_floating();
    }

    else if (match('e') || match('E')) {
        return scan_exponent();
    }

    return add_token(INTEGER);
}

Token scan_hex() {
    if (!isxdigit(peek())) {
        return add_token(ERROR);
    }

    while (isxdigit(peek())) {
        advance();
    }

    return add_token(INTEGER);
}

Token scan_zero() {
    if (match('.')) {
        return scan_floating();
    }

    if (match('x') || match('X')) {
        return scan_hex();
    }

    return add_token(INTEGER);
}

//------------------------------------- Strings --------------------------------------------------

void handle_x_sequence() {
    char hex1 = advance();
    char hex2 = advance();
    if (isxdigit(hex1) && isxdigit(hex2)) {
        char hex_str[3] = {hex1, hex2, '\0'};
        char hex_char = (char) strtol(hex_str, NULL, 16);
        i -= 2;
        replace(hex_char);
    }
}

void handle_escape_sequence() {
    i--;
    advance();
    switch (c) {
        case 'n':
            replace('\n');
            break;
        case 't':
            replace('\t');
            break;
        case 'r':
            replace('\r');
            break;
        case '"':
            replace('"');
            break;
        case '\\':
            replace('\\');
            break;
        case 'x':
            handle_x_sequence();
            break;
        default:
            break;
    }
}

Token scan_normal_string() {
    while (c != '"' && c != EOF && c != '\n') {
        if (c == '\\') {
            handle_escape_sequence();
        }
        advance();
    }

    if (c == '"') {
        replace('\0');
        return add_token(STRING);
    }

    return add_token(ERROR);
}

Token scan_multiline_string() {
    int count = 0;

    while (isspace(peek()) && peek() != '\n') {
        advance();
        reset_buffer();
    }

    while (true) {
        advance();

        if (c == EOF) {
            return add_token(ERROR);
        }

        if (c == '"') {
            count++;
            if (count == 3) {
                i -= 3;
                buffer[i] = '\0';
                while (i > 0 && isspace(buffer[i-1])) {
                    i--;
                }
                buffer[i] = '\0';
                return add_token(STRING);
            }
        }

        else {
            count = 0;
        }
    }
}

Token scan_string() {
    reset_buffer();
    advance();

    if (c == '"') {
        if (match('"')) {
            reset_buffer();
            return scan_multiline_string(); // MULTI-LINE STRING
        }

        reset_buffer();
        return add_token(STRING); // EMPTY STRING
    }

    return scan_normal_string(); // NORMAL STRING
}

//------------------------------------- Operands --------------------------------------

Token scan_operator(char op) {
    switch (op) {
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

        case '{': return add_token(BLOCK_START);
        case '}': return add_token(BLOCK_END);
        case '(': return add_token(BRACKET_START);
        case ')': return add_token(BRACKET_END);
        case '.': return add_token(DOT);
        case ',': return add_token(COMMA);
        case '+': return add_token(PLUS);
        case '-': return add_token(MINUS);
        case '*': return add_token(MULTIPLY);
        case '/': return add_token(DIVIDE);
        case ':': return add_token(COLON);
        case '?': return add_token(QUESTION);
        default:  return add_token(ERROR);
    }
}

//================================================================================================
//                                      FINITE STATE MACHINE
//================================================================================================

Token get_token() {

//------------------------------------- Reset ----------------------------------------------------

    reset_buffer();
    advance();

//------------------------------------- Ignore whitespace ----------------------------------------

    while (isspace(c) && c != '\n') {
        reset_buffer();
        advance();
    }

//------------------------------------- End of file ----------------------------------------------

    if (c == EOF) {
        return add_token(EOF_TOKEN);
    }

//------------------------------------- Comments -------------------------------------------------

    if (c == '/') {
        return scan_slash();
    }

//------------------------------------- New line -------------------------------------------------

    if (c == '\n') {
        if (was_new_line) {
            return get_token();
        }
        was_new_line = true;
        return add_token(NEW_LINE);
    }
    was_new_line = false;

//------------------------------------- Keywords, ID ---------------------------------------------

    if (isalpha(c)) {
        return scan_identifier();
    }

//------------------------------------- Global ID ------------------------------------------------

    else if (c == '_') {
        if (match('_')) {
            return scan_global_id();
        }
        else {
            return add_token(ERROR);
        }
    }

//------------------------------------- Numbers --------------------------------------------------

    else if (isdigit(c)) {
        if (c == '0') {
            return scan_zero();
        } else {
            return scan_number();
        }
    }

//------------------------------------- Strings --------------------------------------------------

    else if (c == '"') {
        return scan_string();
    }

//------------------------------------- Singles and Doubles --------------------------------------

    else {
        return scan_operator(c);
    }
}

//================================================================================================
//                                      DEBUG
//================================================================================================

void print_token(Token token) {
    switch (token.type){
        case ID:            fprintf(output, "ID"); break;
        case GLOBAL_ID:     fprintf(output, "GLOBAL_ID"); break;
        case CLASS:         fprintf(output, "CLASS"); break;
        case IF:            fprintf(output, "IF"); break;
        case ELSE:          fprintf(output, "ELSE"); break;
        case IS:            fprintf(output, "IS"); break;
        case NULL_KEYWORD:  fprintf(output, "NULL_KEYWORD"); break;
        case RETURN:        fprintf(output, "RETURN"); break;
        case VAR:           fprintf(output, "VAR"); break;
        case WHILE:         fprintf(output, "WHILE"); break;
        case IFJ:           fprintf(output, "IFJ"); break;
        case STATIC:        fprintf(output, "STATIC"); break;
        case IMPORT:        fprintf(output, "IMPORT"); break;
        case FOR:           fprintf(output, "FOR"); break;
        case NUM_TYPE:      fprintf(output, "NUM_TYPE"); break;
        case STR_TYPE:      fprintf(output, "STR_TYPE"); break;
        case NULL_TYPE:     fprintf(output, "NULL_TYPE"); break;
        case BOOL_TYPE:     fprintf(output, "BOOL_TYPE"); break;
        case INTEGER:       fprintf(output, "INTEGER"); break;
        case FLOATING:      fprintf(output, "FLOATING"); break;
        case STRING:        fprintf(output, "STRING"); break;
        case BOOLEAN:       fprintf(output, "BOOLEAN"); break;
        case BLOCK_START:   fprintf(output, "BLOCK_START"); break;
        case BLOCK_END:     fprintf(output, "BLOCK_END"); break;
        case BRACKET_START: fprintf(output, "BRACKET_START"); break;
        case BRACKET_END:   fprintf(output, "BRACKET_END"); break;
        case COLON:         fprintf(output, "COLON"); break;
        case QUESTION:      fprintf(output, "QUESTION"); break;
        case DOT:           fprintf(output, "DOT"); break;
        case COMMA:         fprintf(output, "COMMA"); break;
        case PLUS:          fprintf(output, "PLUS"); break;
        case MINUS:         fprintf(output, "MINUS"); break;
        case MULTIPLY:      fprintf(output, "MULTIPLY"); break;
        case DIVIDE:        fprintf(output, "DIVIDE"); break;
        case EQUAL:         fprintf(output, "EQUAL"); break;
        case EQUAL_EQUAL:   fprintf(output, "EQUAL_EQUAL"); break;
        case LESS:          fprintf(output, "LESS"); break;
        case LESS_EQUAL:    fprintf(output, "LESS_EQUAL"); break;
        case MORE:          fprintf(output, "MORE"); break;
        case MORE_EQUAL:    fprintf(output, "MORE_EQUAL"); break;
        case NOT:           fprintf(output, "NOT"); break;
        case NOT_EQUAL:     fprintf(output, "NOT_EQUAL"); break;
        case AND:           fprintf(output, "AND"); break;
        case OR:            fprintf(output, "OR"); break;
        case NEW_LINE:      fprintf(output, "NEW_LINE"); break;
        case EOF_TOKEN:     fprintf(output, "EOF"); break;
        case ERROR:         fprintf(output, "ERROR"); break;
        default:            fprintf(output, "UNKNOWN"); break;
    }

    if (token.type == INTEGER) {
        fprintf(output, "(%d)", token.value.integer);
    }
    else if (token.type == FLOATING) {
        fprintf(output, "(%f)", token.value.floating);
    }
    else if (token.type == STRING) {
        fprintf(output, "(%s)", token.value.string);
    }
    else if (token.type == ID) {
        fprintf(output, "(%s)", token.value.string);
    }
    else if (token.type == GLOBAL_ID) {
        fprintf(output, "(%s)", token.value.string);
    }
    else if (token.type == BOOLEAN) {
        fprintf(output, "(%s)", token.value.boolean ? "true" : "false");
    }

    fprintf(output, "\n");
}

void parser_function(bool debug) {
    Token token;
    do {
        token = get_token();
        if (debug) {
            print_token(token);
        }
    } while (token.type != EOF_TOKEN);
}
