#ifndef __INDEXED_PALETTE_IMG_H__
#define __INDEXED_PALETTE_IMG_H__

#include "simple_img_system.h"

void *indexed_palette_img_create(img_type_t type, uint16_t w, uint16_t h,uint32_t *data_size);

void indexed_palette_img_clear_to_color(img_t *pimg, uint32_t c);

void indexed_palette_img_fill_vtable(img_t *pimg);

#endif
