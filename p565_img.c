#include <malloc.h>
#include <stdio.h>
#include <stdint.h>

#include "p565_img.h"

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

static const uint8_t _LUT5[32] = 
{
  0, 8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98, 106,
  115, 123, 131, 139, 148, 156, 164, 172, 180, 189, 197,
  205, 213, 222, 230, 238, 246, 255
};

static const uint8_t _LUT6[64] =
{
 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80,
 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149,
 153, 157, 161, 165, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 214,
 218, 222, 226, 230, 234, 238, 242, 246, 250, 255
};

static const uint8_t _LUT3[8] = 
{
  0, 36, 72, 109, 145, 182, 218, 255
};

static const uint8_t _LUT2[4] = 
{
  0, 85, 170, 255
};

static p565_pixel_t _color2p565(uint32_t c)
{
  p565_pixel_t ret;

  ret.b = COLOR2P565_B(c);
  ret.g = COLOR2P565_G(c);
  ret.r = COLOR2P565_R(c);

  return ret;
}

static p332_pixel_t _color2p332(uint32_t c)
{
  p332_pixel_t ret;

  ret.b = COLOR2P332_B(c);
  ret.g = COLOR2P332_G(c);
  ret.r = COLOR2P332_R(c);

  return ret;
}

img_t *p565_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c)
{
  img_t    *ret;
  uint32_t  data_size;
  p565_pixel_t initial_color_565;
  p332_pixel_t initial_color_332;
  int i;

  switch(type)
  {
  case img_type_p565:
    data_size = w * h * sizeof(p565_pixel_t);
  break;

  case img_type_p332:
    data_size = w * h * sizeof(p332_pixel_t);
  break;
  }

  ret = (img_t*) malloc(sizeof(img_t) +  data_size);
  
  if(ret)
  {
    ret->img_type = type;
    ret->width = w;
    ret->height = h;
    ret->extra = (void*) NULL;
    ret->data_size = data_size;
    p565_img_fill_vtable(ret);

    /* pre-fill the image with the background color */
    switch (ret->img_type)
    {
      case img_type_p565:
        initial_color_565 = _color2p565(c);    
        for(i = 0; i < ret->width * ret->height; i++)
          ((p565_pixel_t *)(ret->data))[i] = initial_color_565;
      break;

      case img_type_p332:
        initial_color_332 = _color2p332(c);
        for(i = 0; i < ret->width * ret->height; i++)
          ((p332_pixel_t *)(ret->data))[i] = initial_color_332;
      break;
    }
  }

  return ret;
}


void p565_img_clear_to_color(img_t *pimg, uint32_t c)
{
  int i;
  p565_pixel_t initial_color_565;
  p332_pixel_t initial_color_332;

}


void p565_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    free(pimg);
  }
}

void p565_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint32_t pixel_index;
  p565_pixel_t our_color_565;
  p332_pixel_t our_color_332;

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      switch (pimg->img_type)
      {
        case img_type_p565:
          our_color_565 = _color2p565(c);
          pixel_index = y * pimg->width + x;
          ((p565_pixel_t *)pimg->data)[pixel_index] = our_color_565;
        break;

        case img_type_p332:
          our_color_332 = _color2p332(c);
          pixel_index = y * pimg->width + x;
          ((p332_pixel_t *)pimg->data)[pixel_index] = our_color_332;
        break;
      }
    }
  }
}


void p565_img_dump_stats(img_t *pimg, const char* title)
{
  int i;

  if(pimg)
  {
    printf("%s, Width:  %d, ", title, pimg->width);
    printf("Height: %d\n", pimg->height);
  }
}

uint32_t p565_img_getpixelclamped (img_t *pimg, uint16_t x, uint16_t y)
{
  p565_pixel_t *data_565;
  p565_pixel_t pel_565;
  p332_pixel_t *data_332;
  p332_pixel_t pel_332;

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  switch(pimg->img_type)
  {
    case img_type_p565:
      data_565 = (p565_pixel_t*) (pimg->data);
      pel_565 = data_565[y*pimg->width + x];

      return RGB(_LUT5[(pel_565.r)], _LUT6[(pel_565.g)], _LUT5[(pel_565.b)]);

    case img_type_p332:
      data_332 = (p332_pixel_t*) (pimg->data);
      pel_332 = data_332[y*pimg->width + x];

      return RGB(_LUT3[(pel_332.r)], _LUT3[(pel_332.g)], _LUT2[(pel_332.b)]);
  }
}


void p565_img_fill_vtable(img_t *pimg)
{
  pimg->plot_func = p565_img_plot;
  pimg->destroy_func = p565_img_destroy;
  pimg->dump_stats_func = p565_img_dump_stats;
  pimg->get_pixel_func = p565_img_getpixelclamped;
}
