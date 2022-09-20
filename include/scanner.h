#ifndef SCANNER_H
#define SCANNER_H

#include <token.h>


void scanner_init(const char* buf);
void scanner_destroy(void);
void scanner_clear_cache(void);
uint8_t scanner_is_eof(void);


/*
 *  Returns 0 if no more tokens, otherwise
 *  1.
 *
 */
uint8_t scan(struct Token* out);

#endif
