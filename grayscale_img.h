#ifndef __GRAYSCALE_IMG_H__
#define __GRAYSCALE_IMG_H__

#include "simple_img_system.h"

img_t *grayscale_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c);
void grayscale_img_fill_vtable(img_t *pimg);
#endif
