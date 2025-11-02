/**
 * @file scanner.c
 * @author Petr David Lanca
 * @brief Scanner for tokenizing input
 * @date 2025-10-01
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "scanner.h"

FILE *file;                 // Input file from stdin
FILE *out;                  // Output file to build/tokens.txt
char c;                     // Current character
char buffer[BUFFER_SIZE];   // Current token buffer
int i = 0;                  // Buffer index
bool was_new_line = false;  // Last generated token was NEW_LINE


// === Character and Buffer handling ========================================
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

    if (token.type == ID || token.type == GLOBAL_ID) {
        strncpy(token.value.string, buffer, i);
        token.value.string[i] = '\0';
    }

    else if (token.type == STRING) {
        strncpy(token.value.string, buffer, i);
        token.value.string[i] = '\0';
    }

    else if (token.type == INTEGER) {
        buffer[i] = '\0';
        token.value.integer = strtol(buffer, NULL, 0);
    }

    else if (token.type == FLOATING) {
        buffer[i] = '\0';
        token.value.floating = atof(buffer);
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


// === Token loops ===========================================
Token single_line_comment() {
    advance();
    while (c != '\n' && c != EOF) {
        c = advance();
    }
    ungetc(c, file);
    return get_token();
}

Token multi_line_comment() {
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

Token handle_slash() {
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

// Helper functions for better code organization
bool is_identifier_char(char ch) {
    return isalnum(ch) || ch == '_';
}

Token scan_identifier() {
    // Continue reading while we have valid identifier characters
    while (is_identifier_char(peek()) && i < BUFFER_SIZE - 1) {
        c = advance();
    }
    
    // Null-terminate the buffer for string comparison
    buffer[i] = '\0';
    
    // Check if it's a keyword or regular identifier
    return add_token(lookup_keyword(buffer));
}

Token scan_global_id() {
    // Advance past the second underscore
    c = advance();
    
    // Continue reading while we have valid identifier characters
    while (is_identifier_char(peek()) && i < BUFFER_SIZE - 1) {
        c = advance();
    }
    
    // Null-terminate the buffer for string comparison
    buffer[i] = '\0';
    
    return add_token(GLOBAL_ID);
}

// Number parsing helper functions
bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool is_hex_digit(char ch) {
    return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool scan_digits() {
    if (!is_digit(peek())) {
        return false;
    }
    while (is_digit(peek()) && i < BUFFER_SIZE - 1) {
        c = advance();
    }
    return true;
}

Token scan_exponent() {
    if (!match('e') && !match('E')) {
        return add_token(FLOATING);
    }
    
    c = advance(); // consume 'e' or 'E'
    
    if (match('+') || match('-')) {
        c = advance();
    }
    
    if (scan_digits()) {
        return add_token(FLOATING);
    } else {
        return add_token(ERROR);
    }
}

Token scan_decimal_number() {
    // Scan integer part (already scanned first digit)
    while (is_digit(peek()) && i < BUFFER_SIZE - 1) {
        c = advance();
    }
    
    // Check for decimal point
    if (match('.')) {
        c = advance();
        if (!scan_digits()) {
            return add_token(ERROR);
        }
        return scan_exponent();
    }
    
    // Check for exponent
    if (match('e') || match('E')) {
        c = advance();
        if (match('+') || match('-')) {
            c = advance();
        }
        if (scan_digits()) {
            return add_token(FLOATING);
        } else {
            return add_token(ERROR);
        }
    }
    
    return add_token(INTEGER);
}

Token scan_hex_number() {
    c = advance(); // consume 'x'
    
    if (!is_hex_digit(peek())) {
        return add_token(ERROR);
    }
    
    while (is_hex_digit(peek()) && i < BUFFER_SIZE - 1) {
        c = advance();
    }
    
    return add_token(INTEGER);
}

Token scan_zero() {
    if (match('.')) {
        c = advance();
        if (!scan_digits()) {
            return add_token(ERROR);
        }
        return scan_exponent();
    }
    
    if (match('x') || match('X')) {
        return scan_hex_number();
    }
    
    return add_token(INTEGER);
}

// String parsing helper functions
void add_char_to_buffer(char ch) {
    if (i < BUFFER_SIZE - 1) {
        buffer[i] = ch;
        i++;
    }
}

bool handle_escape_sequence() {
    c = advance(); // consume the character after backslash
    
    switch (c) {
        case 'n':
            add_char_to_buffer('\n');
            return true;
        case 't':
            add_char_to_buffer('\t');
            return true;
        case 'r':
            add_char_to_buffer('\r');
            return true;
        case '"':
            add_char_to_buffer('"');
            return true;
        case '\\':
            add_char_to_buffer('\\');
            return true;
        case 'x': {
            char hex1 = advance();
            char hex2 = advance();
            if (is_hex_digit(hex1) && is_hex_digit(hex2)) {
                char hex_str[3] = {hex1, hex2, '\0'};
                char hex_char = (char) strtol(hex_str, NULL, 16);
                add_char_to_buffer(hex_char);
                return true;
            } else {
                return false; // Error
            }
        }
        default:
            return false; // Error
    }
}

Token scan_regular_string() {
    reset_buffer();
    
    while (c != '"' && c != EOF && c != '\n' && i < BUFFER_SIZE - 1) {
        if (c == '\\') {
            if (!handle_escape_sequence()) {
                return add_token(ERROR);
            }
            // Don't call advance() here - handle_escape_sequence already advanced
        } else {
            add_char_to_buffer(c);
            c = advance();
        }
    }
    
    if (c == '"') {
        buffer[i] = '\0';
        return add_token(STRING);
    } else {
        return add_token(ERROR);
    }
}

Token scan_multiline_string() {
    reset_buffer();
    int quote_count = 0;
    
    while (i < BUFFER_SIZE - 1) {
        c = advance();
        
        if (c == EOF) {
            return add_token(ERROR);
        }
        
        if (c == '"') {
            quote_count++;
            if (quote_count == 3) {
                buffer[i] = '\0';
                return add_token(STRING);
            }
        } else {
            // Add any accumulated quotes to buffer
            while (quote_count > 0) {
                add_char_to_buffer('"');
                quote_count--;
            }
            add_char_to_buffer(c);
        }
    }
    
    return add_token(ERROR); // Buffer overflow or unterminated
}

Token scan_string() {
    c = advance(); // consume opening quote
    
    if (c == '"') {
        c = advance();
        if (c == '"') {
            // Triple quote - multiline string
            return scan_multiline_string();
        } else {
            // Empty string
            reset_buffer();
            return add_token(STRING);
        }
    } else {
        // Regular string - c is already the first character
        return scan_regular_string();
    }
}

// Operator parsing helper functions
Token scan_single_char_operator(char op) {
    switch (op) {
        case '{': return add_token(BLOCK_START);
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
        default:  return add_token(ERROR);
    }
}

Token scan_comparison_operator(char op) {
    switch (op) {
        case '=':
            return add_token(match('=') ? EQUAL_EQUAL : EQUAL);
        case '<':
            return add_token(match('=') ? LESS_EQUAL : LESS);
        case '>':
            return add_token(match('=') ? MORE_EQUAL : MORE);
        case '!':
            return add_token(match('=') ? NOT_EQUAL : NOT);
        default:
            return add_token(ERROR);
    }
}

Token scan_logical_operator(char op) {
    switch (op) {
        case '&':
            return add_token(match('&') ? AND : ERROR);
        case '|':
            return add_token(match('|') ? OR : ERROR);
        default:
            return add_token(ERROR);
    }
}

Token scan_operator(char op) {
    // Check for comparison operators
    if (op == '=' || op == '<' || op == '>' || op == '!') {
        return scan_comparison_operator(op);
    }
    
    // Check for logical operators
    if (op == '&' || op == '|') {
        return scan_logical_operator(op);
    }
    
    // Check for single character operators
    return scan_single_char_operator(op);
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
    if (c == EOF) {
        return add_token(EOF_TOKEN);
    }

    // Single and multi-line comments, division operator
    if (c == '/') {
        return handle_slash();
    }

    // New line
    if (c == '\n') {
        if (was_new_line) {
            return get_token();
        }
        was_new_line = true;
        return add_token(NEW_LINE);
    }
    was_new_line = false;

    // Keyword, ID
    if (isalpha(c)) {
        return scan_identifier();
    }

    // Global ID
    else if (c == '_') {
        if (match('_')) {
            return scan_global_id();
        }
        else {
            return add_token(ERROR);
        }
    }

    // Numbers (integers, floats, hex, exponents)
    else if (isdigit(c)) {
        if (c == '0') {
            return scan_zero();
        } else {
            return scan_decimal_number();
        }
    }

    // Strings (regular, empty, multiline)
    else if (c == '"') {
        return scan_string();
    }

    // Operators and punctuation
    else {
        return scan_operator(c);
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
    } else if (token.type == FLOATING) {
        fprintf(out, "       FLT[%f]", token.value.floating);
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

int main()
{
    file = stdin;

    out = fopen("../build/tokens.txt", "w");
    if (!out) {return 1;}

    prototype_parser_function();

    fclose(out);

    return 0;
}

