#!/bin/sh
# make mingw binary

CC="i486-mingw32-gcc" AR="i486-mingw32-ar" make mingw=1 $@
