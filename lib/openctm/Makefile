###############################################################################
# Product:     OpenCTM
# File:        Makefile.linux
# Description: Makefile for Linux systems (should work on most Un*x-like
#              systems with gcc, e.g. OpenSolaris).
###############################################################################
# Copyright (c) 2009-2010 Marcus Geelnard
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not
#     be misrepresented as being the original software.
#
#     3. This notice may not be removed or altered from any source
#     distribution.
###############################################################################

LZMADIR = liblzma
CTMFLAG = -DOPENCTM_BUILD -I$(LZMADIR) -DLZMA_PREFIX_CTM

TARGET = libopenctm.a

OBJS = openctm.o \
       stream.o \
       compressRAW.o \
       compressMG1.o \
       compressMG2.o

LZMA_OBJS = Alloc.o \
            LzFind.o \
            LzmaDec.o \
            LzmaEnc.o \
            LzmaLib.o

SRCS = openctm.c \
       stream.c \
       compressRAW.c \
       compressMG1.c \
       compressMG2.c

LZMA_SRCS = $(LZMADIR)/Alloc.c \
            $(LZMADIR)/LzFind.c \
            $(LZMADIR)/LzmaDec.c \
            $(LZMADIR)/LzmaEnc.c \
            $(LZMADIR)/LzmaLib.c

all: $(TARGET)

clean:
	${RM} -f ../$(TARGET)
	${RM} -f $(OBJS)
	${RM} -f $(LZMA_OBJS)
	${RM} -f ../../include/openctm.h

$(TARGET): $(OBJS) $(LZMA_OBJS)
	${AR} rcs ../$(TARGET) $^
	cp openctm.h ../../include/

%.o: %.c
	$(CC) $(CFLAGS) $(CTMFLAG) -c $^ -o $@

%.o: $(LZMADIR)/%.c
	$(CC) $(CFLAGS) $(CTMFLAG) -c $^ -o $@

