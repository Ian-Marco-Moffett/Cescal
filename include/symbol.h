#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>


// Structural types.
typedef enum {
    S_VAR,
    S_FUNC
} SYMBOL_STYPE;

// Primitive types.
typedef enum {
    P_U8,
    P_U16,
    P_U32,
    P_U64,
    P_NONE,
    P_INVALID,
} SYMBOL_PTYPE;


typedef enum {
    FUNC_NAKED = (1 << 0),
    FUNC_PUBLIC = (1 << 1),
} FUNC_FLAGS;


struct Symbol {
    const char* name;
    SYMBOL_STYPE stype;
    SYMBOL_PTYPE ptype;
    FUNC_FLAGS func_flags;
};


void sym_tbl_init(void);

/*
 *  Push a global symbol.
 *
 *  Returns: Slot number of new symbol.
 *
 */
uint64_t addglob(const char* name, SYMBOL_STYPE stype, SYMBOL_PTYPE ptype);


/*
 * Locates glob @param name and returns slot number.
 *
 */

int64_t findglob(const char* name);
void destroy_symtbl(void);


extern struct Symbol* g_globsymTable;

#endif
