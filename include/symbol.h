#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>


void sym_tbl_init(void);

/*
 *  Push a global symbol.
 *
 *  Returns: Slot number of new symbol.
 *
 */
uint64_t addglob(const char* name);


/*
 * Locates glob @param name and returns slot number.
 *
 */

int64_t findglob(const char* name);

void destroy_symtbl(void);


extern char** g_globsymTable;

#endif
