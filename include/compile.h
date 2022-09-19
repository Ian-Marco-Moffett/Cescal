#ifndef COMPILE_H
#define COMPILE_H

#include <ast.h>
#include <regs.h>

// Compiles to asm and doesn't assemble (-asmonly).
#define COMPILE_FLAG_ASMONLY        (1 << 0)

// Compiles to an object file (-c).
#define COMPILE_FLAG_OBJ            (1 << 1)

// Compiles as a freestanding binary (-freestanding)
#define COMPILE_FLAG_FREESTANDING   (1 << 2)

void compile_start(void);
void compile_end(void);
uint32_t get_flags(void);
REG_T ast_gen(struct ASTNode* n, int reg, int parent_ast_top);

#endif
