DL_WINDOW_DISABLE_LOG := 0

INCF		= DL
PREF		= dl

SOURCE		= ${PREF}Window.c
INCLUDES	= -I../../include
TARGET		= lib${INCF}window.a
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))


all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} -DDL_WINDOW_DISABLE_LOG=${DL_WINDOW_DISABLE_LOG} \
	${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs   	../${TARGET} $^
	mkdir -p    	../../include/${INCF}
	cp ${PREF}Window.h	../../include/${INCF}/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/${INCF}/${PREF}Window.h
