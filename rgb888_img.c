#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "rgb888_img.h"

#define COLOR_B(c) ((c) & 0xFF)
#define COLOR_G(c) (((c) & 0xFF00) >> 8)
#define COLOR_R(c) (((c) & 0xFF0000) >> 16)

typedef struct _rgb888_pixel_t
{
  uint8_t b;
  uint8_t g;
  uint8_t r;
} rgb888_pixel_t;

img_t *rgb888_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c)
{
  img_t    *ret = (img_t*)NULL;
  uint32_t  data_size;
  rgb888_pixel_t initial_color;
  int i;

  switch(type)
  {
  case img_type_rgb888:
    data_size = w * h * sizeof(rgb888_pixel_t);
  break;

  default:
    printf("Erroneous image type passed into %s line %d\n", __FILE__,__LINE__);
    return ret;
  }

  ret = (img_t*) malloc(sizeof(img_t) +  data_size);
  
  if(ret)
  {
    ret->img_type = type;
    ret->width = w;
    ret->height = h;
    ret->num_channels = 3;
    ret->extra = (void*) NULL;
    ret->data_size = data_size;
    rgb888_img_fill_vtable(ret);

    /* pre-fill the image with the background color */
    initial_color.b = COLOR_B(c);
    initial_color.g = COLOR_G(c);
    initial_color.r = COLOR_R(c);

    switch (ret->img_type)
    {
      case img_type_rgb888:    
        for(i = 0; i < ret->width * ret->height; i++)
          ((rgb888_pixel_t *)(ret->data))[i] = initial_color;
      break;

      default:
        printf("Erroneous image type passed in to %s line %d\n", __FILE__, __LINE__);
        free(ret);
        ret = (img_t*)NULL;
    }
  }

  return ret;
}


void rgb888_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    free(pimg);
  }
}

void rgb888_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint32_t pixel_index;
  rgb888_pixel_t our_color;

  our_color.b = COLOR_B(c);
  our_color.g = COLOR_G(c);
  our_color.r = COLOR_R(c);

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      switch (pimg->img_type)
      {
        case img_type_rgb888:
          pixel_index = y * pimg->width + x;
          ((rgb888_pixel_t *)pimg->data)[pixel_index] = our_color;
        break;

        default:
          printf("Erroneous img type passed into %s line %d\n", __FILE__, __LINE__);
      }
    }
  }
}


void rgb888_img_dump_stats(img_t *pimg, const char* title)
{
  int i;

  if(pimg)
  {
    printf("%s, Width:  %d, ", title, pimg->width);
    printf("Height: %d\n", pimg->height);
  }
}

uint32_t rgb888_img_getpixelclamped (img_t *pimg, uint16_t x, uint16_t y)
{
  rgb888_pixel_t *data;
  rgb888_pixel_t pel;

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  switch(pimg->img_type)
  {
    case img_type_rgb888:
      pel = ((rgb888_pixel_t*)(pimg->data))[y*pimg->width + x];

      return RGB(pel.r, pel.g, pel.b);

    default:
      printf("Erroneous img type passed in to %s line %d\n", __FILE__, __LINE__);
      return RGB(0,0,0);
  }
}


void rgb888_img_fill_vtable(img_t *pimg)
{
  pimg->plot_func = rgb888_img_plot;
  pimg->destroy_func = rgb888_img_destroy;
  pimg->dump_stats_func = rgb888_img_dump_stats;
  pimg->get_pixel_func = rgb888_img_getpixelclamped;
}
