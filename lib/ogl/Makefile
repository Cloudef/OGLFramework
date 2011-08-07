SOURCE		= framework.c alloc.c sceneobject.c vbo.c			\
				  ibo.c texture.c logfile_wrapper.c	material.c  \
				  atlas.c camera.c SDL/SDL_rotozoom.c				\
				  $(patsubst %.c,%,$(wildcard render/*.c)) 		\
				  $(patsubst %.c,%,$(wildcard geometry/*.c))    \
				  $(patsubst %.c,%,$(wildcard skeletal/*.c))		\
				  $(patsubst %.c,%,$(wildcard import/*.c)) 		\
				  import/mmd_import/mmd.c								\
              $(patsubst %.c,%,$(wildcard packer/*.c))
INCLUDES		= -I. -I../../include
TARGET		= libogl.a
OBJ			= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	mkdir -p 				../../include/ogl
	cp ogl.h					../../include/ogl/
	cp framework.h 		../../include/ogl/
	cp core.h 				../../include/ogl/
	cp sceneobject.h 		../../include/ogl/
	cp vbo.h					../../include/ogl/
	cp ibo.h					../../include/ogl/
	cp scolor.h				../../include/ogl/
	cp config.h				../../include/ogl/
	cp texture.h			../../include/ogl/
	cp material.h			../../include/ogl/
	cp atlas.h				../../include/ogl/
	cp camera.h				../../include/ogl/
	mkdir -p 				../../include/ogl/skeletal
	cp skeletal/*.h		../../include/ogl/skeletal/

clean:
	${RM}  ${OBJ}
	${RM}  ../${TARGET}
	${RM} -r ../../include/ogl

