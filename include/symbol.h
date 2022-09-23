#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>


// Structural types.
typedef enum {
    S_VAR,
    S_FUNC
} SYMBOL_STYPE;

typedef enum {
    SCOPE_LOCAL,
    SCOPE_GLOBAL,
} SYMBOL_SCOPE;

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
    SYMBOL_SCOPE scope;
    FUNC_FLAGS func_flags;
    struct Symbol* local_symtbl;
    uint64_t n_local_symbols;
    uint32_t n_args;                // For functions.
    uint64_t rbp_off;               // RBP offset of variable.
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
int64_t find_loc(struct Symbol* glob, const char* local_name);
void destroy_symtbl(void);
void create_local_symtbl(struct Symbol* glob);

/*
 *  Appends a local symbol to the symbol table.
 *  Returns symbol id.
 *
 */

uint64_t local_symtbl_append(struct Symbol* glob, const char* name, SYMBOL_PTYPE ptype);

extern struct Symbol* g_globsymTable;

#endif
