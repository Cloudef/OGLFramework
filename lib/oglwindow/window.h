#ifndef WINDOW_H
#define WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int  glCreateWindow(int, int, int, unsigned int);
struct SDL_Surface *glGetSDLSurface(void);
void glCloseWindow(void);
void glSwapBuffers(void);

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */

/* EoF */
