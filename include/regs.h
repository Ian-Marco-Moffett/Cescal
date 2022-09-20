#ifndef REGS_H
#define REGS_H

#include <stdint.h>
#include <stdio.h>


typedef int8_t REG_T;


void reg_init(FILE* out);
void regs_free(void);
REG_T reg_load(int64_t value);
REG_T reg_add(REG_T r1, REG_T r2);
REG_T reg_mul(REG_T r1, REG_T r2);
REG_T reg_sub(REG_T r1, REG_T r2);
REG_T reg_div(REG_T r1, REG_T r2);

REG_T equal(REG_T r1, REG_T r2);
REG_T notequal(REG_T r1, REG_T r2);
REG_T lessthan(REG_T r1, REG_T r2);
REG_T greaterthan(REG_T r1, REG_T r2);
REG_T lessequal(REG_T r1, REG_T r2);
REG_T greaterequal(REG_T r1, REG_T r2);
REG_T reg_store_glob(REG_T r, int64_t nameslot);                // Store reg into glob.
REG_T load_glob(int64_t nameslot);                         // Load glob into reg.

void reg_printint(REG_T r);
void reg_printstr(REG_T r);


#endif
