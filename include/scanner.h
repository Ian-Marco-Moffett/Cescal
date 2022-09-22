#ifndef SCANNER_H
#define SCANNER_H

#include <token.h>


void scanner_init(const char* buf);
void scanner_destroy(void);
void scanner_clear_cache(void);
uint8_t scanner_is_eof(void);
void scanner_peek(struct Token* out);
void scanner_change_buf(char* buf);
void scanner_restore_buf(void);
char* scan_dil(char dil, char end, uint8_t* is_end);

/*
 *  Returns 0 if no more tokens, otherwise
 *  1.
 *
 */
uint8_t scan(struct Token* out);

#endif
