#ifndef WINDOW_H
#define WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif   

int  glCreateWindow(int, int, int, unsigned int);
void glCloseWindow();
void glSwapBuffers();

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */

/* EoF */
