SOURCE		= input.c
INCLUDES		= -I.
TARGET		= libinput.a
OBJ			= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	cp input.h 			../../include/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/input.h
