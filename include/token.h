#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stddef.h>

#define IS_DIGIT_ASCII(x) x >= '0' && x <= '9'


typedef enum {
    TT_EOF,             // 0.
    TT_PLUS,            // 1.                    
    TT_MINUS,           // 2.
    TT_STAR,            // 3.
    TT_SLASH,           // 4.
    TT_EQEQ,            // 5.
    TT_NOTEQUAL,        // 6.
    TT_LESSTHAN,        // 7.
    TT_GREATERTHAN,     // 8.
    TT_LESSTHANEQ,      // 9.
    TT_GREATERTHANEQ,   // 10.
    TT_INTLIT,          // 11.
    TT_SEMI,            // 12.
    TT_EQUALS,          // 13.
    TT_ID,              // 14.
    TT_FUNC,            // 15.
    TT_LBRACE,          // 16.
    TT_RBRACE,          // 17.
    TT_NONE,            // 18.
    TT_LPAREN,          // 19.
    TT_RPAREN,          // 20.
    TT_NOT,             // 21.
    TT_U8,              // 22.
    TT_U16,             // 23.
    TT_U32,             // 24.
    TT_U64,             // 25.
    TT_STRINGLIT,       // 26.
    TT_IF,              // 27.
    TT_WHILE,           // 28.
    TT_RETURN,          // 29.
    TT_LBRACKET,        // 30.
    TT_RBRACKET,        // 31.
    TT_EXTERN,          // 32.

    // Compiler built in related stuff.
    TT_PUTS, 
    TT_ASM,
    TT_INVALID,
    

    // NOT USED IN PARSING.
    TT_INCLUDE,
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
