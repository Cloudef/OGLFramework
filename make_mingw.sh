#!/bin/sh
# make mingw binary

CC="i486-mingw32-gcc" AR="i486-mingw32-ar" make CFLAGS="" mingw=1 $@
