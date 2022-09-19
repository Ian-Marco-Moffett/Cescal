#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stddef.h>

#define IS_DIGIT_ASCII(x) x >= '0' && x <= '9'


typedef enum {
    TT_INTLIT,          // 0.
    TT_SLASH,           // 1.
    TT_PLUS,            // 2.
    TT_MINUS,           // 3.
    TT_STAR,            // 4.
    TT_FUNC,            // 5.
    TT_ID,              // 6.
    TT_EQUALS,          // 7.
    TT_GREATERTHAN,     // 8.
    TT_LESSTHAN,        // 9.
    TT_LBRACE,          // 10.
    TT_RBRACE,          // 11.
    TT_NULL,            // 12.
    TT_SEMI,            // 13.
    TT_LPAREN,          // 14.
    TT_RPAREN,          // 15.

    // Compiler built in related stuff.
    TT_LINUX_PUTS,
    
    // Other.
    TT_EOF,
    TT_INVALID
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
