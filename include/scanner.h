#ifndef SCANNER_H
#define SCANNER_H

#include <token.h>


void scanner_init(const char* buf);


/*
 *  Returns 0 if no more tokens, otherwise
 *  1.
 *
 */
uint8_t scan(struct Token* out);

#endif
