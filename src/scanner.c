#include <scanner.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define SHOULD_IGNORE(c) c == ' ' || c == '\t' || c == '\n' || c == '\r'


static const char* in_buf = NULL;
static size_t in_buf_index = 0;
static size_t line_number = 1;

void scanner_init(const char* buf) {
    in_buf = buf;
}


static int64_t scanint(void) {
    size_t n = 0;
    char* buf = calloc(n + 1, sizeof(char));

    while (IS_DIGIT_ASCII(in_buf[in_buf_index])) {
        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = in_buf[in_buf_index++];
    }

    int64_t tmp = atoi(buf);
    free(buf);
    return tmp;
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
        default:
            printf("ERROR: Invalid token '%c' while scanning!\n", tok);
        case 0:
            out->type = TT_EOF;
            return 0;
    }
    
    out->line_number = line_number;
    ++in_buf_index;
    return 1;
}
