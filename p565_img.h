#ifndef __P565_IMG_H__
#define __P565_IMG_H__
#include <stdint.h>

#include "simple_img_system.h"

#define RGB(r, g, b) ((r)<<16 | ((g)<<8) | (b))

#define COLOR2P565_B(c) ((((c) & 0xFF) * 249 + 1014) >> 11)
#define COLOR2P565_G(c) (((((c) & 0xFF00) >> 8) * 253 + 505) >> 10)
#define COLOR2P565_R(c) (((((c) & 0xFF0000) >> 16) * 249 + 1014) >> 11)

#define COLOR2P332_B(c) (((c) & 0xFF) >> 6)
#define COLOR2P332_G(c) ((((c) & 0xFF00) >> 8) >> 5)
#define COLOR2P332_R(c) ((((c) & 0xFF0000) >> 16) >> 5)

typedef struct _p565_pixel_t
{
  uint16_t b : 5;
  uint16_t g : 6;
  uint16_t r : 5;
} p565_pixel_t;

typedef struct _p332_pixel_t
{
  uint8_t b : 2;
  uint8_t g : 3;
  uint8_t r : 3;
} p332_pixel_t;


void *p565_img_create(img_t *pimg, uint32_t c);
void p565_img_fill_vtable(img_t *pimg);
#endif
