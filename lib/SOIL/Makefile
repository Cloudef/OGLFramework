SOURCE		= SOIL.c image_DXT.c image_helper.c stb_image_aug.c
INCLUDES	= -I../../include
TARGET		= libSOIL.a
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	cp SOIL.h ../../include/

clean:
	${RM} -f ${OBJ}
	${RM} -f ../${TARGET}
	${RM} -f ../../include/SOIL.h
