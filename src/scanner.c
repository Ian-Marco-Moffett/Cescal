#include <scanner.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <ctype.h>
#include <string.h>

#define SHOULD_IGNORE(c) c == ' ' || c == '\t' || c == '\n' || c == '\r'


static const char* in_buf = NULL;
static size_t in_buf_index = 0;
static size_t line_number = 1;
static const char* last_alloc = NULL;
static uint8_t is_eof = 0;

void scanner_init(const char* buf) {
    in_buf = buf;
}


void scanner_destroy(void) {
    if (last_alloc != NULL) {
        free((void*)last_alloc);
    }

    last_alloc = NULL;
}


uint8_t scanner_is_eof(void) {
    return is_eof;
}



static int64_t scanint(void) {
    size_t n = 0;
    char* buf = calloc(n + 1, sizeof(char));

    while (IS_DIGIT_ASCII(in_buf[in_buf_index])) {
        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = in_buf[in_buf_index++];
    }

    switch (in_buf[in_buf_index]) {
        case '+':
        case '-':
        case '/':
        case '*':
        case ';':
            break;
        default:
            if (SHOULD_IGNORE(in_buf[in_buf_index])) {
                break;
            }

            printf("ERROR: Invalid token '%c' while scanning! (line %d)\n", in_buf[in_buf_index], line_number);
            free(buf);
            panic();
    }
    
    --in_buf_index;
    int64_t tmp = atoi(buf);
    free(buf);
    return tmp;
}


// Scan identifier.
static const char* scanid(void) {
    size_t n = 0;
    char* buf = calloc(n + 3, sizeof(char));
    
    char ch = in_buf[in_buf_index++];
    while (isalpha(ch) || ch == '_' && ch) {
        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = ch;
        ch = in_buf[in_buf_index++];
    } 
    
    last_alloc = buf;
    return buf;
}


static TOKEN_TYPE id_get_tok(const char* id) {
    if (strcmp(id, "func") == 0) {
        return TT_FUNC;
    } else if (strcmp(id, "__linux_puts") == 0) {
        return TT_LINUX_PUTS;
    } else if (strcmp(id, "null") == 0) {
        return TT_NULL;
    }

    return TT_ID;
}


uint8_t scan(struct Token* out) {
    char tok = in_buf[in_buf_index]; 
    while (SHOULD_IGNORE(tok)) {
        if (tok == '\n')
            ++line_number;

        tok = in_buf[++in_buf_index];
    }

    switch (tok) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            out->type = TT_INTLIT;
            out->tokstring = NULL;
            out->tokint = scanint();
            break;
        case '/':
            out->type = TT_SLASH;
            out->tokstring = NULL;
            break;
        case '+':
            out->type = TT_PLUS;
            out->tokstring = NULL;
            break;
        case '-':
            out->type = TT_MINUS;
            out->tokstring = NULL;
            break;
        case '*':
            out->type = TT_STAR;
            out->tokstring = NULL;
            break;
        case '=':
            out->type = TT_EQUALS;
            out->tokstring = NULL;
            break;
        case '>':
            out->type = TT_GREATERTHAN;
            out->tokstring = NULL;
            break;
        case '{':
            out->type = TT_LBRACE;
            out->tokstring = NULL;
            break;
        case '}':
            out->type = TT_RBRACE;
            out->tokstring = NULL;
            break;
        case ';':
            out->type = TT_SEMI;
            out->tokstring = NULL;
            break;
        case '(':
            out->type = TT_LPAREN;
            out->tokstring = NULL;
            break;
        case ')':
            out-.type = TT_RPAREN;
            out->tokstring = NULL;
        default: 
            // Identifier or keyword.
            if (isalpha(tok) || tok == '_') {
                out->tokstring = scanid();
                out->type = id_get_tok(out->tokstring);
                --in_buf_index;
                break;
            }
            
            printf("ERROR: Invalid token '%c' while scanning! (line %d)\n", tok, line_number);
            panic();
        case 0:
            out->type = TT_EOF;
            out->tokstring;
            is_eof = 1;
            return 0;
    }
    
    out->line_number = line_number;
    ++in_buf_index;
    return 1;
}
