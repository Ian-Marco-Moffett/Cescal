#ifndef REGS_H
#define REGS_H

#include <stdint.h>
#include <stdio.h>

#define RREG_RET 4


typedef int8_t REG_T;


void reg_init(FILE* out);
void regs_free(void);
void reg_free(REG_T reg);
REG_T reg_load(int64_t value);
REG_T reg_add(REG_T r1, REG_T r2);
REG_T reg_mul(REG_T r1, REG_T r2);
REG_T reg_sub(REG_T r1, REG_T r2);
REG_T reg_div(REG_T r1, REG_T r2);
REG_T reg_shr(REG_T r1, REG_T r2);
REG_T reg_shl(REG_T r1, REG_T r2);

REG_T equal(REG_T r1, REG_T r2);
REG_T notequal(REG_T r1, REG_T r2);
REG_T lessthan(REG_T r1, REG_T r2);
REG_T greaterthan(REG_T r1, REG_T r2);
REG_T lessequal(REG_T r1, REG_T r2);
REG_T greaterequal(REG_T r1, REG_T r2);
REG_T reg_store_glob(REG_T r, int64_t nameslot);
REG_T reg_store_var(REG_T r, int64_t nameslot);                     // Stores reg into a var.
REG_T load_var(int64_t nameslot);                                   // Loads var into reg.
void reg_printint(REG_T r);
void reg_printstr(REG_T r);
const char* get_rreg_str(REG_T r);
const char* get_breg_str(REG_T r);
const char* get_dreg_str(REG_T r);
const char* get_wreg_str(REG_T r);


#endif
