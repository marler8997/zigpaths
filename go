#!/usr/bin/env sh
set -ex
#zig build-exe zigpaths.c
gcc zigpaths.c path.c doesfileexists.c unicode.c env.c -Werror-implicit-function-declaration
