#ifndef DL_WINDOW_H
#define DL_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int                 dlCreateWindow(int width, int height, int bitdepth, unsigned int sdl_flags);
void                dlWindowSetMode(int width, int height, int bitdepth, unsigned int sdl_flags);
char*               dlWindowGetError(void);
struct SDL_Surface *dlGetSDLSurface(void);
void                dlCloseWindow(void);
void                dlSwapBuffers(void);

#ifdef __cplusplus
}
#endif

#endif /* DL_WINDOW_H */

/* EoF */
