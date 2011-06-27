SOURCE		= logfile.c logcolor.c
INCLUDES		= -I.
TARGET		= liblogfile.a
OBJ			= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	cp logfile.h 			../../include/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/logfile.h
