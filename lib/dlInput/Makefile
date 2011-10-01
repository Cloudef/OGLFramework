INCF		= DL
PREF		= dl

SOURCE		= ${PREF}Input.c
INCLUDES	= -I.
TARGET		= lib${INCF}input.a
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs  ../${TARGET} $^
	mkdir -p	  ../../include/${INCF}
	cp ${PREF}Input.h ../../include/${INCF}/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/${INCF}/${PREF}Input.h
