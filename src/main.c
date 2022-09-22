#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <scanner.h>
#include <stddef.h>
#include <parser.h>
#include <symbol.h>
#include <string.h>
#include <compile.h>


static char* g_buf = NULL;
static FILE* g_fp = NULL;
static uint32_t flags = 0;
static const char* current_fname = NULL;            // Current included path being parsed.
const char* target_fname;                           // File that's being compiled.


void set_current_filename(const char* fname) {
    if (fname == NULL) {
        free((char*)current_fname);  // current_fname made by strdup().
    }

    current_fname = fname;
}

void panic(void) {
    if (g_buf != NULL) free(g_buf);
    if (g_fp != NULL) fclose(g_fp);
    scanner_destroy();
    destroy_symtbl();

    if (current_fname != NULL) {
        printf("(Error occured in included file %s)\n", current_fname);
        free((char*)current_fname);         // current_fname made by strdup().
    }

    exit(1);
}


uint32_t get_flags(void) {
    return flags;
}

static void compile(FILE* fp) {
    // Get file size.
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate memory for file buffer.
    char* buf = calloc(fsize + 1, sizeof(char));
    fread(buf, sizeof(char), fsize, fp);
    
    sym_tbl_init();
    scanner_init(buf);
    parse();
    free(buf);
    destroy_symtbl();
}


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("cesc: Too few arguments!\n");
        return 1;
    } 

    int n_files = 0;

    // Check flags and arg count.
    // TODO: Allow more filenames.
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-asmonly") == 0) {
            flags |= COMPILE_FLAG_ASMONLY;
        } else if (strcmp(argv[i], "-freestanding") == 0) {
           flags |= COMPILE_FLAG_FREESTANDING;
        } else if (strcmp(argv[i], "-kessykernel") == 0) {
            flags |= COMPILE_FLAG_KESSYKERNEL;
        } else if (strcmp(argv[i], "-c") == 0) {
            flags |= COMPILE_FLAG_OBJ;
        } else if (argv[i][0] != '-' && n_files < 2) {
            ++n_files;
        } else {
            printf("cesc: Too many paths (for now)!\n");
            return 1;
        }
    }

    // Get filenames.
    for (int i = 1; i < argc; ++i) { 
        if (argv[i][0] == '-') {
            continue;
        } 

        FILE* fp = fopen(argv[i], "r");

        if (!(fp)) {
            perror(argv[i]);
            return 1;
        }

        target_fname = argv[i];
        compile(fp);
    }

    return 0;
}
