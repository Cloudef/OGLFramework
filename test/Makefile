DIRS = $(wildcard test_*)

all: ${DIRS}

${DIRS}:
	${MAKE} -C $@ clean && ${MAKE} -C $@ CFLAGS="${CFLAGS}" GL_LIBS="${GL_LIBS}"

.PHONY: ${DIRS}
