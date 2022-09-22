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
    TT_SHR,             // 11.
    TT_SHL,             // 12.
    TT_INTLIT,          // 13.
    TT_SEMI,            // 14.
    TT_EQUALS,          // 15.
    TT_ID,              // 16.
    TT_FUNC,            // 17.
    TT_LBRACE,          // 18.
    TT_RBRACE,          // 19.
    TT_NONE,            // 20.
    TT_LPAREN,          // 21.
    TT_RPAREN,          // 22.
    TT_NOT,             // 23.
    TT_U8,              // 24.
    TT_U16,             // 25.
    TT_U32,             // 26.
    TT_U64,             // 27.
    TT_STRINGLIT,       // 28.
    TT_IF,              // 29.
    TT_WHILE,           // 30.
    TT_RETURN,          // 31.
    TT_LBRACKET,        // 32.
    TT_RBRACKET,        // 33.
    TT_EXTERN,          // 34. 

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
