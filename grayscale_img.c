#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "grayscale_img.h"

#define COLOR2G8(c) (((c) & 0xFF00) >> 8)
#define COLOR2G4(c) ((((c) & 0xFF00) >> 8) >> 4)

const uint8_t _LUT4[16] =
{
    0,  17,  34,  51,
   68,  85, 102, 119,
  136, 153, 170, 187,
  204, 221, 238, 255
};

img_t *grayscale_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c)
{
  img_t    *ret = (img_t*)NULL;
  uint32_t  data_size;
  int i;
  uint8_t   initial_color;

  switch(type)
  {
  case img_type_grayscale8:
    data_size = w * h * sizeof(uint8_t);
  break;

  case img_type_grayscale4:
    data_size = (w & 0xFFFE) * h * sizeof(uint8_t)/2;
  break;

  default:
    printf("Erroneous image type passed into %s line %d\n", __FILE__,__LINE__);
    return ret;
  }

  ret = (img_t*) malloc(sizeof(img_t) +  data_size);
  
  if(ret)
  {
    ret->img_type = type;
    ret->width = w & 0xFFFE;  // force to even sized width
    ret->height = h;
    ret->num_channels = 1;
    ret->extra = (void*) NULL;
    ret->data_size = data_size;
    grayscale_img_fill_vtable(ret);

    /* pre-fill the image with the background color */
    switch (ret->img_type)
    {
      case img_type_grayscale8:
        initial_color = COLOR2G8(c);   
      break;

      case img_type_grayscale4:
        initial_color = COLOR2G4(c);
        initial_color = initial_color | (initial_color << 4);
      break;

      default:
        printf("Erroneous image type passed in to %s line %d\n", __FILE__, __LINE__);
        free(ret);
        ret = (img_t*)NULL;
    }

    printf("initial color: 0x%02X\n", initial_color);
    memset((void*)ret->data, initial_color, data_size); 
  }
  return ret;
}


void grayscale_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    free(pimg);
  }
}

void grayscale_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint8_t  our_color;
  uint8_t  hilo;
  uint32_t pixel_index;

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      switch (pimg->img_type)
      {
        case img_type_grayscale8:
          our_color = COLOR2G8(c);
          pixel_index = y * pimg->width + x;
          ((uint8_t *)pimg->data)[pixel_index] = our_color;
        break;

        case img_type_grayscale4:
          our_color = COLOR2G4(c);
          hilo = x&1;
          pixel_index = y * pimg->width/2 + x/2;
          if(!hilo)
          {
            ((uint8_t *)pimg->data)[pixel_index] &= 0xF0;
            ((uint8_t *)pimg->data)[pixel_index] |= our_color;
          }
          else
          {
            ((uint8_t *)pimg->data)[pixel_index] &= 0x0F;
            ((uint8_t *)pimg->data)[pixel_index] |= our_color<<4;
          }
          
        break;

        default:
          printf("Erroneous img type passed into %s line %d\n", __FILE__, __LINE__);
      }
    }
  }
}


void grayscale_img_dump_stats(img_t *pimg, const char* title)
{
  int i;

  if(pimg)
  {
    printf("%s, Width:  %d, ", title, pimg->width);
    printf("Height: %d\n", pimg->height);
  }
}

uint32_t grayscale_img_getpixelclamped (img_t *pimg, uint16_t x, uint16_t y)
{
  uint8_t pel;
  uint8_t hilo;

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  switch(pimg->img_type)
  {
    case img_type_grayscale8:
      pel = pimg->data[y*pimg->width + x];
      return RGB(pel, pel, pel);

    case img_type_grayscale4:
      hilo = x&1;
      pel = pimg->data[y*pimg->width/2 + x/2];
      if(!hilo)
      {
        return RGB(_LUT4[pel&0x0F],_LUT4[pel&0x0F],_LUT4[pel&0x0F]);
      }
      else
      {
        return RGB(_LUT4[(pel&0xF0)>>4],_LUT4[(pel&0xF0)>>4],_LUT4[(pel&0xF0)>>4]);
      }

    default:
      printf("Erroneous img type passed in to %s line %d\n", __FILE__, __LINE__);
      return RGB(0,0,0);
  }
}


void grayscale_img_fill_vtable(img_t *pimg)
{
  pimg->plot_func = grayscale_img_plot;
  pimg->destroy_func = grayscale_img_destroy;
  pimg->dump_stats_func = grayscale_img_dump_stats;
  pimg->get_pixel_func = grayscale_img_getpixelclamped;
}
