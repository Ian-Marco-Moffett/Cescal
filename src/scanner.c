#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <scanner.h>
#include <panic.h>
#include <preprocessor.h>

#define SHOULD_IGNORE(c) c == ' ' || c == '\t' || c == '\n' || c == '\r'
#define PEEK_AHEAD(n) in_buf[in_buf_index + n]


static char* old_in_buf = NULL;
static size_t old_in_buf_idx = 0;
static size_t old_line_num = 0;
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


void scanner_peek(struct Token* out) {
    size_t tmp = in_buf_index;
    scan(out);
    in_buf_index = tmp;
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
        case ')':
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
    while (isalpha(ch) || ch == '_' || IS_DIGIT_ASCII(ch)) {
        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = ch;
        ch = in_buf[in_buf_index++];
    }
    
    --in_buf_index;
    last_alloc = buf;
    return buf;
}


static const char* scanstr(void) {
    size_t n = 0;
    char* buf = calloc(n + 3, sizeof(char));
    size_t start_line_num = line_number;
    
    char ch = in_buf[++in_buf_index];
    while (ch != '"') {
        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = ch;
        ch = in_buf[++in_buf_index];

        if (ch == 0) {
            free(buf);
            printf("ERROR: EOF found before double quote while scanning string literal, line %d\n", start_line_num);
            panic();
        }
    }
    
    last_alloc = buf;
    return buf;
}


static TOKEN_TYPE id_get_tok(const char* id) {
    if (strcmp(id, "func") == 0) {
        return TT_FUNC;
    } else if (strcmp(id, "puts") == 0) {
        return TT_PUTS;
    } else if (strcmp(id, "none") == 0) {
        return TT_NONE;
    } else if (strcmp(id, "u8") == 0) {
        return TT_U8;
    } else if (strcmp(id, "u16") == 0) {
        return TT_U16;
    } else if (strcmp(id, "u32") == 0) {
        return TT_U32;
    } else if (strcmp(id, "u64") == 0) {
        return TT_U64;
    } else if (strcmp(id, "if") == 0) {
        return TT_IF;
    } else if (strcmp(id, "while") == 0) {
        return TT_WHILE;
    } else if (strcmp(id, "return") == 0) {
        return TT_RETURN;
    } else if (strcmp(id, "__asm") == 0) {
        return TT_ASM;
    } else if (strcmp(id, "extern") == 0) {
        return TT_EXTERN;
    } else if (strcmp(id, "__naked") == 0) {
        return TT_NAKED;
    } else if (strcmp(id, "public") == 0) {
        return TT_PUBLIC;
    }

    return TT_ID;
}


static TOKEN_TYPE preprocess_get_type(const char* directive) {
    if (strcmp(directive, "include") == 0) {
        return TT_INCLUDE;
    }

    return TT_INVALID;
}


void scanner_clear_cache(void) {
    last_alloc = NULL;
}


/*
 *  Scan until @param end is reached and split by @param dil.
 *
 *
 */
char* scan_dil(char dil, char end, uint8_t* is_end) {
    size_t n = 0;
    char* buf = calloc(n + 3, sizeof(char));
    size_t start_line_num = line_number; 
    char ch = in_buf[++in_buf_index];

    uint8_t is_trimming_lspaces = 1;

    while (1) {
        if (ch == dil) {
            *is_end = 0;
            return buf;
        } else if (ch == end) {
            *is_end = 1;
            ++in_buf_index;
            return buf;
        }

        if ((ch == ' ' || ch == '\t') && is_trimming_lspaces) {
            ch = in_buf[++in_buf_index];
            continue;
        } else {
            is_trimming_lspaces = 0;
        }

        buf = realloc(buf, sizeof(char) * (n + 2));
        buf[n++] = ch;
        ch = in_buf[++in_buf_index];

        if (ch == 0) {
            free(buf);
            printf("ERROR: EOF found before '%c' while scanning, line %d\n", end, start_line_num);
            panic();
        }
    }
    
    last_alloc = buf;
    return buf;
}

void scanner_change_buf(char* buf) {
    old_in_buf = (char*)in_buf;
    old_in_buf_idx = in_buf_index;
    old_line_num = line_number;
    in_buf = buf;
    in_buf_index = 0;
}

void scanner_restore_buf(void) {
    in_buf = old_in_buf;
    in_buf_index = old_in_buf_idx;
    line_number = old_line_num;
    is_eof = 0;
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
        case '[':
            out->type = TT_LBRACKET;
            out->tokstring = NULL;
            break;
        case ']':
            out->type = TT_RBRACKET;
            out->tokstring = NULL;
            break;
        case '"':
            out->type = TT_STRINGLIT;
            out->tokstring = scanstr();
            break;
        case '@':
            ++in_buf_index;
            TOKEN_TYPE directive_type = preprocess_get_type(scanid());

            if (directive_type == TT_INCLUDE) {
                if (PEEK_AHEAD(1) != '"') {
                    printf("ERROR: Expected \"filename.cekl\", line %d\n", line_number);
                    panic();
                }

                ++in_buf_index;
                const char* included_file = scanstr();
                preprocessor_include(included_file, (char**)&in_buf, line_number);
            } 
           
            ++in_buf_index;
            scan(out);
            --in_buf_index;
            break;
        case '!':
            if (PEEK_AHEAD(1) == '=') {
                out->type = TT_NOTEQUAL;
                in_buf_index += 1;
            } else {
                out->type = TT_NOT;
            }
            out->tokstring = NULL;
            break;
        case '=':
            if (PEEK_AHEAD(1) == '=') {
                out->type = TT_EQEQ;
                in_buf_index += 1;
            } else {
                out->type = TT_EQUALS;
            }
            out->tokstring = NULL;
            break;
        case '>':
            if (PEEK_AHEAD(1) == '=') {
                out->type = TT_GREATERTHANEQ;
                in_buf_index += 1;
            } else if (PEEK_AHEAD(1) == '>') {
                out->type = TT_SHR;
                in_buf_index += 1;
            } else {
                out->type = TT_GREATERTHAN;
            }
            out->tokstring = NULL;
            break;
        case '<':
            if (PEEK_AHEAD(1) == '=') {
                out->type = TT_LESSTHANEQ;
                in_buf_index += 1;
            } else if (PEEK_AHEAD(1) == '<') {
                out->type = TT_SHL;
                in_buf_index += 1;
            } else {
                out->type = TT_LESSTHAN;
            }

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
            out->type = TT_RPAREN;
            out->tokstring = NULL;
            break;
        case 0:
            out->type = TT_EOF;
            out->tokstring;
            is_eof = 1;
            return 0;
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
    }
    
    out->line_number = line_number;
    ++in_buf_index;
    return 1;
}
