# Needed global defines
CFLAGS  += -DGL_GLEXT_PROTOTYPES -D_GNU_SOURCE

# If you want to use your own CFLAGS
# This disables all default optimizations,
# but does still append defines for library.
OWN_CFLAGS	:= 0

# Standard libraries for projects
# Order : Rightmost = first
GL_LIBS += -lDL -lDLwindow -lDLinput -llogfile -lSOIL -lkazmath -lm

# 0 = OpenGL
# 1 = GLES 1.0
# 2 = GLES 2.0
GL 		:= 0

# Use vertex color arrays
VERTEX_COLOR	:= 0

# Use index buffers.
# Needed if you are using unsigned short indices
# and need models with many vertices.
# Enabled for GLES 1.0
INDEX_BUFFERS 	:= 0

# Release ?
release		:= 0

# 32bit binary ?
# Enabled automatically when using either
# GLES1 or GLES2, also enabled for mingw
x86 		:= 0

# Compiling using mingw?
mingw 		:= 0

# Build tests?
BUILD_TESTS	:= 1

# OpenCTM loader ( lib: openctm )
OPENCTM		:= 1

# MikuMikuDance PMD loader ( lib: none )
PMD		:= 1

# Convert SJIS strigns from PMD file to utf-8 ( lib: iconv )
ICONV_SJIS_PMD	:= 1

# Assimp loader ( lib: assimp )
ASSIMP 		:= 1

# Global CFLAGS
ifeq (${OWN_CFLAGS}, 0)
     CFLAGS  += -fsingle-precision-constant
endif

# Release settings
ifeq (${release}, 1)
     ifeq (${OWN_CFLAGS},0)
          # Some of these flags cause crash on mingw binary, maybe it's just wine?
          ifeq (${CC},/usr/local/angstrom/arm/bin/arm-angstrom-linux-gnueabi-gcc)
	       # ARM
          else ifeq (${wingw}, 0)
               CFLAGS += -Wall -O2 -funroll-loops -ffast-math -fomit-frame-pointer -s
          else
               CFLAGS += -Wall -O2 -funroll-loops -ffast-math -s
          endif
     endif
     GL_LIBS += -s
else
     ifeq (${OWN_CFLAGS}, 0)
     	CFLAGS += -Wall -O0 -g -DDEBUG
     else
   	CFLAGS += -DDEBUG
     endif
endif

# mingw:
# link SDLmain and SDL
# gcc:
# just SDL
ifeq (${mingw}, 1)
     GL_LIBS += -static-libgcc -mwindows
     GL_LIBS += -lmingw32 -lSDLmain -lSDL

     # Enable x86
     x86 = 1
else
     # -lSDL
     GL_LIBS += `pkg-config --libs sdl`
endif

# OpenCTM
ifeq (${OPENCTM}, 1)
     CFLAGS  += -DWITH_OPENCTM=1
     GL_LIBS += -lopenctm
else
     CFLAGS  += -DWITH_OPENCTM=0
endif

# PMD
ifeq (${PMD}, 1)
     CFLAGS += -DWITH_PMD=1
     ifeq (${ICONV_SJIS_PMD}, 1)
          CFLAGS += -DICONV_SJIS_PMD=1
     else
          CFLAGS += -DICONV_SJIS_PMD=0
     endif
else
     CFLAGS += -DWITH_PMD=0
endif

# ASSIMP
ifeq (${ASSIMP}, 1)
     CFLAGS  += -DWITH_ASSIMP=1
     GL_LIBS += -lassimp
else
     CFLAGS  += -DWITH_ASSIMP=0
endif

# Check if we are compiling ARM binary
ifeq (${CC},/usr/local/angstrom/arm/bin/arm-angstrom-linux-gnueabi-gcc)
     ifeq (${GL},0)
          GL = 1
     endif
endif

# Check which GL mode we are on
ifeq (${GL},0)
     ifeq (${mingw}, 0)
   	  # -lGL -lGLEW
          GL_LIBS += `pkg-config --libs gl glew`
     else
          GL_LIBS += -lopengl32 -lglew32
     endif
     ifeq (${x86},1)
          CFLAGS += -m32
     endif
else ifeq (${GL},1)
     GL_LIBS += -lGLES_CM -lEGL -lX11
     CFLAGS  += -DGLES1
     INDEX_BUFFERS = 1
     ifeq (${x86},1)
          CFLAGS += -m32
     endif
else ifeq (${GL}, 2)
     GL_LIBS += -lGLESv2 -lEGL -lX11
     CFLAGS  += -DGLES2
     ifeq (${x86},1)
          CFLAGS += -m32
     endif
endif

# Index Buffers
ifeq (${INDEX_BUFFERS}, 1)
     CFLAGS += -DUSE_BUFFERS=1
else
     CFLAGS += -DUSE_BUFFERS=0
endif

# Vertex Colors
ifeq (${VERTEX_COLOR}, 1)
     CFLAGS += -DVERTEX_COLOR=1
else
     CFLAGS += -DVERTEX_COLOR=0
endif

all: dl test

openctm:
ifeq (${OPENCTM}, 1)
	@${MAKE} -C lib/openctm 	CFLAGS="${CFLAGS}"
endif

kazmath:
	@${MAKE} -C lib/kazmath    	CFLAGS="${CFLAGS} -std=c99"

SOIL:
	@${MAKE} -C lib/SOIL		CFLAGS="${CFLAGS}"

logfile:
	@${MAKE} -C lib/logfile 	CFLAGS="${CFLAGS}"

dlWindow: logfile
	@${MAKE} -C lib/dlWindow	CFLAGS="${CFLAGS}"

dlInput:
	@${MAKE} -C lib/dlInput 	CFLAGS="${CFLAGS}"

dl: kazmath SOIL openctm logfile
	@${MAKE} -C lib/dl 		CFLAGS="${CFLAGS} -std=c99"

test: dl dlWindow dlInput
ifeq (${BUILD_TESTS}, 1)
	@${MAKE} -C test 		CFLAGS="${CFLAGS}" GL_LIBS="${GL_LIBS}" mingw="${mingw}"
endif

clean:
	@${MAKE} -C lib/kazmath		clean
	@${MAKE} -C lib/SOIL		clean
	@${MAKE} -C lib/logfile		clean
	@${MAKE} -C lib/dl 		clean
	@${MAKE} -C lib/dlWindow 	clean
	@${MAKE} -C lib/dlInput 	clean
	@${MAKE} -C lib/openctm		clean
	@${MAKE} -C test		clean
