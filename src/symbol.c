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


void destroy_symtbl(void) {
    for (int i = 0; i < g_globCount; ++i) {
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
    return slot;
}
