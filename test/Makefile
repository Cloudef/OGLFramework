DIRS = $(wildcard test_*)

all : ${DIRS}
	for c in ${DIRS}; do \
	${MAKE} -C $${c} CFLAGS="${CFLAGS}" GL_LIBS="${GL_LIBS}"; \
	done

clean : ${DIRS}
	for c in ${DIRS}; do \
	${MAKE} -C $${c} clean; \
	done
