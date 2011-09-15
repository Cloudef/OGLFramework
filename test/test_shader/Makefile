SOURCE		= shader.c
INCLUDES	= -I../../include
LIB		= -L../../lib
TARGET		= shader.bin
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${CC} ${CFLAGS} -o $@ $^ ${GL_LIBS} ${LIB}
	mv ${TARGET} ../bin/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../bin/${TARGET}
