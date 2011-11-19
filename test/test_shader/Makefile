SOURCE		= shader.c
INCLUDES	= -I../../include
LIB		= -L../../lib
TARGET		= shader
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

ifeq (${mingw}, 1)
	FTARGET = $(addsuffix .exe, $(TARGET))
else
	FTARGET = $(addsuffix .run, $(TARGET))
endif

all: ${FTARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${FTARGET}: ${OBJ}
	${CC} ${CFLAGS} -o $@ $^ ${GL_LIBS} ${LIB}
	mv ${FTARGET} ../bin/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../bin/${TARGET}.exe
	${RM} -f ../bin/${TARGET}.run
