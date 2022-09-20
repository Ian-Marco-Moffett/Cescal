#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stddef.h>


// @param current_file_data: contents of current file.
void preprocessor_include(const char* file, char** current_file_data, size_t cur_line);


#endif
