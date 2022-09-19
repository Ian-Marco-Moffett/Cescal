CC = gcc
CFILES = $(shell find src/ -name "*.c")

bin/cesc: $(CFILES)
	@ mkdir -p bin/
	$(CC) $^ -Iinclude/	\
        -Werror=implicit \
        -Werror=implicit-function-declaration \
        -Werror=implicit-int \
        -Werror=int-conversion \
        -Werror=incompatible-pointer-types \
        -Werror=int-to-pointer-cast -o $@
