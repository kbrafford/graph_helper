#ifndef __P565_IMG_H__
#define __P565_IMG_H__

#include "simple_img_system.h"

void *p565_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t *data_size);
void p565_img_fill_vtable(img_t *pimg);
#endif
