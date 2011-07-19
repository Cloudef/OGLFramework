#ifndef GL_CAMERA_H
#define GL_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

/* sceneobject struct */
typedef struct glCamera_t
{
 	unsigned int refCounter;
} glCamera;

glCamera*   glNewCamera( void );			               /* Allocate camera */
glCamera*   glCopyCamera( glCamera *src );	         /* Copy camera  */
glCamera*   glRefCamera( glCamera *src );	            /* Reference camera  */
int         glFreeCamera( glCamera *object );	      /* Free camera */

#ifdef __cplusplus
}
#endif

#endif /* GL_CAMERA_H */

/* EoF */
