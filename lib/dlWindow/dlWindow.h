#ifndef DL_WINDOW_H
#define DL_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int                 dlCreateWindow(int, int, int, unsigned int);
struct SDL_Surface *dlGetSDLSurface(void);
void                dlCloseWindow(void);
void                dlSwapBuffers(void);

#ifdef __cplusplus
}
#endif

#endif /* DL_WINDOW_H */

/* EoF */
