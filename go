#!/usr/bin/env sh
set -ex
#zig build-exe zigpaths.c
gcc -g -std=c99 -o zigpaths zigpaths.c path.c doesfileexists.c unicode.c env.c -Werror-implicit-function-declaration
./zigpaths
