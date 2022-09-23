#include <symbol.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Symbol* g_globsymTable = NULL;
static uint64_t g_globCount = 0;


int64_t findglob(const char* name) {
    for (int i = 0; i < g_globCount; ++i) {
        if (strcmp(g_globsymTable[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}


int64_t find_loc(struct Symbol* glob, const char* local_name) {
    for (int i = 0; i < glob->n_local_symbols; ++i) {
        if (strcmp(glob->local_symtbl[i].name, local_name) == 0) {
            return i;
        }
    }

    return -1;
}


void destroy_symtbl(void) {
    for (int i = 0; i < g_globCount; ++i) {
        // Free local symbols.
        if (g_globsymTable[i].local_symtbl != NULL) {
            for (int j = 0; j < g_globsymTable[i].n_local_symbols; ++j) {
                free((char*)g_globsymTable[i].local_symtbl[j].name);
            }

            free(g_globsymTable[i].local_symtbl);
        }

        free((char*)g_globsymTable[i].name);
    }
    
    if (g_globsymTable != NULL) {
        free(g_globsymTable);
        g_globsymTable = NULL;
    }
}

void sym_tbl_init(void) {
    g_globCount = 0;
    
    if (g_globsymTable != NULL) { 
        destroy_symtbl();
    }

    g_globsymTable = malloc(sizeof(struct Symbol));
}


void create_local_symtbl(struct Symbol* glob) {
    glob->local_symtbl = malloc(sizeof(struct Symbol));
    glob->n_local_symbols = 0;
    glob->rbp_off = 0;
}


uint64_t local_symtbl_append(struct Symbol* glob, const char* name, SYMBOL_PTYPE ptype) {
    struct Symbol s = {
        .name = strdup(name),
        .ptype = ptype,
        .stype = S_VAR,
        .scope = SCOPE_LOCAL
    };

    switch (ptype) {
        case P_U8:
            glob->rbp_off += 1;
            break;
        case P_U16:
            glob->rbp_off += 2;
            break;
        case P_U32:
            glob->rbp_off += 4;
            break;
        case P_U64:
            glob->rbp_off += 8;
            break;
    }

    s.rbp_off = glob->rbp_off;
    glob->local_symtbl[glob->n_local_symbols++] = s;
    glob->local_symtbl = realloc(glob->local_symtbl, sizeof(struct Symbol) * (glob->n_local_symbols + 2));
    return glob->n_local_symbols - 1;
}


uint64_t addglob(const char* name, SYMBOL_STYPE stype, SYMBOL_PTYPE ptype) {
    int64_t slot;
    // If symbol already exists, return existing slot.
    if ((slot = findglob(name)) != -1) {
        return slot;
    }
    
    slot = g_globCount++;
    g_globsymTable = realloc(g_globsymTable, sizeof(struct Symbol) * g_globCount + 1);
    g_globsymTable[slot].name = strdup(name);
    g_globsymTable[slot].stype = stype;
    g_globsymTable[slot].ptype = ptype;
    g_globsymTable[slot].scope = SCOPE_GLOBAL;
    g_globsymTable[slot].local_symtbl = NULL;
    g_globsymTable[slot].rbp_off = 0;
    g_globsymTable[slot].n_args = 0;
    return slot;
}
