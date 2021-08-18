#!/bin/sh

SRCS="./src/fat_io_lib/release/fat*.c ./src/espack.c ./src/crc32.c ./src/espack_linux.c"

rm -f ./espack64 ./espack32
gcc -O2 -Wall -Wno-unused-function -D_FILE_OFFSET_BITS=64 $SRCS  -I./src/ -I./src/fat_io_lib/release -o espack64
gcc -O2 -m32 -Wall -Wno-unused-function -D_FILE_OFFSET_BITS=64 $SRCS  -I./src/ -I./src/fat_io_lib/release -o espack32
