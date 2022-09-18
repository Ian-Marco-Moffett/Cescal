#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stddef.h>

#define IS_DIGIT_ASCII(x) x >= '0' && x <= '9'


typedef enum {
    TT_INTLIT,
    TT_SLASH,
    TT_PLUS,
    TT_MINUS,
    TT_STAR,
    TT_EOF,
} TOKEN_TYPE;


struct Token {
    TOKEN_TYPE type;
    size_t line_number;

    union {
        const char* tokstring;
        int64_t tokint;
    };
};


#endif
