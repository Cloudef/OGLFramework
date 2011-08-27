/* Enable/Disable features of framework */

#ifndef GL_CONFIG_H
#define GL_CONFIG_H

#define GL_LOG_FILE    "opengl.log" /* NULL for no log */

#define GL_MAX_BUFFERS 5   /* If you need to use unsigned short indices
                              (GLES 1.0) you might want to control this
                              variable for maximum index buffers */

/* Specify type for animation nodes,
 * change this if you have more than USHRT_MAX frames per animation,
 * or more than USHRT_MAX animations */
#ifndef GL_NODE_TYPE
   #define GL_NODE_TYPE unsigned short
#endif

/* Keyframe animation */
#ifndef USE_KEYFRAME_ANIMATION
   #define USE_KEYFRAME_ANIMATION 0
#endif

/* Vertex color support */
#ifndef VERTEX_COLOR
   #define VERTEX_COLOR 0
#endif

/* Enable/Disable formats */

/* OpenCTM http://openctm.sourceforge.net/ */
#ifndef WITH_OCTM
   #define WITH_OCTM    0
#endif

/* MikuMikuDance PMD loader */
#ifndef WITH_PMD
   #define WITH_PMD        1
   #define ICONV_SJIS_PMD  0    /* only if you want that SJIS encoded output in utf-8 */
#endif

/* ASSIMP imports lots of formats */
#ifndef WITH_ASSIMP
   #define WITH_ASSIMP  0
#endif

#endif /* GL_CONFIG_H */

/* EoF */
