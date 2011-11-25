#ifndef DL_SCOLOR_H
#define DL_SCOLOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* dlColor struct */
typedef struct
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
   uint8_t a;
} dlColor;

#ifdef __cplusplus
}
#endif

#endif /* DL_SCOLOR_H */
