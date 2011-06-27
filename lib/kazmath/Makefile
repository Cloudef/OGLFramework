SOURCE		= aabb.c mat3.c mat4.c plane.c quaternion.c utility.c vec2.c vec3.c vec4.c
INCLUDES		= -I. -I../../include
TARGET		= libkazmath.a
OBJ			= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	mkdir -p ../../include/kazmath
	cp *.h ../../include/kazmath

clean:
	${RM}  ${OBJ}
	${RM}  ../${TARGET}
	${RM} -r ../../include/kazmath
