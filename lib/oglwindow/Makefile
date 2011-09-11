SOURCE		= window.c
INCLUDES	= -I../../include
TARGET		= liboglwindow.a
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs   ../${TARGET} $^
	mkdir -p    ../../include/ogl
	cp window.h ../../include/ogl/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/ogl/window.h
