#ifndef __BMP_IMG_H__
#define __BMP_IMG_H__

#include "simple_img_system.h"

img_t *bmp_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c);
void bmp_img_fill_vtable(img_t *pimg);
#endif
