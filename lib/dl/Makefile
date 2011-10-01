INCF		= DL
PREF		= dl

SOURCE		= $(patsubst %.c,%,$(wildcard *.c))		\
		  SDL/SDL_rotozoom.c				\
		  import/mmd_import/mmd.c			\
		  $(patsubst %.c,%,$(wildcard render/*.c)) 	\
		  $(patsubst %.c,%,$(wildcard shader/*.c)) 	\
		  $(patsubst %.c,%,$(wildcard geometry/*.c))    \
		  $(patsubst %.c,%,$(wildcard skeletal/*.c))	\
		  $(patsubst %.c,%,$(wildcard import/*.c)) 	\
		  $(patsubst %.c,%,$(wildcard atlaspacker/*.c))
INCLUDES	= -I. -I../../include
TARGET		= lib${INCF}.a
OBJ		= $(addsuffix .o, $(basename $(SOURCE)))

all: ${TARGET}
	@true

%.o : %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $^ -o $@

${TARGET}: ${OBJ}
	${AR} rcs ../${TARGET} $^
	mkdir -p 		../../include/${INCF}
	cp ${PREF}.h		../../include/${INCF}/
	cp ${PREF}Framework.h 	../../include/${INCF}/
	cp ${PREF}Core.h 	../../include/${INCF}/
	cp ${PREF}Sceneobject.h ../../include/${INCF}/
	cp ${PREF}Vbo.h		../../include/${INCF}/
	cp ${PREF}Ibo.h		../../include/${INCF}/
	cp ${PREF}Scolor.h	../../include/${INCF}/
	cp ${PREF}Config.h	../../include/${INCF}/
	cp ${PREF}Texture.h	../../include/${INCF}/
	cp ${PREF}Material.h	../../include/${INCF}/
	cp ${PREF}Atlas.h	../../include/${INCF}/
	cp ${PREF}Camera.h	../../include/${INCF}/
	mkdir -p 		../../include/${INCF}/shader
	cp shader/*.h		../../include/${INCF}/shader/
	mkdir -p 		../../include/${INCF}/skeletal
	cp skeletal/*.h		../../include/${INCF}/skeletal/

clean:
	${RM}  ${OBJ}
	${RM}  ../${TARGET}
	${RM} -r ../../include/${INCF}

