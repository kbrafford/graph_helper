#ifndef __INDEXED_PALETTE_IMG_H__
#define __INDEXED_PALETTE_IMG_H__
#include <stdio.h>
#include <stdint.h>
#include "simple_img_system.h"

typedef struct _indexed_palette_extra_t
{
  uint32_t palette[4096];
  uint32_t _mrc;
  uint8_t  next_color;
  uint8_t  _mrc_idx;  
  uint16_t palette_max_size;  
} indexed_palette_extra_t;


indexed_palette_extra_t *indexed_palette_img_create(img_t *pimg, uint32_t c);
void indexed_palette_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c);
void indexed_palette_img_fill_vtable(img_t *pimg);

#endif
