#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "simple_img_system.h"
#include "indexed_palette_img.h"

typedef struct _indexed_palette_extra_t
{
  uint32_t palette[4096];
  uint32_t _mrc;
  uint8_t  next_color;
  uint8_t  _mrc_idx;  
  uint16_t palette_max_size;  
} indexed_palette_extra_t;


uint16_t _get_color_idx(img_t *pimg, uint32_t c)
{
  int                      i;
  int                      found = 0;
  indexed_palette_extra_t *extra = (indexed_palette_extra_t*)pimg->extra;

  c &= 0xFFFFFF;

  if(c != extra->_mrc)
  {
    /* see if this is a color we've already seen */
    for(i = 0; i < extra->next_color; i++)
    {
      if (c == extra->palette[i])
      {
        extra->_mrc_idx = i;
        extra->_mrc = c;
        found = 1;
        break;
      }
    }

    /* see if we need to make a new color */
    if((!found)&&(extra->next_color < extra->palette_max_size))
    {
      extra->palette[extra->next_color] = c;
      extra->_mrc = c;
      extra->_mrc_idx = extra->next_color;
      extra->next_color++;
    }
  }

  if(extra->next_color == extra->palette_max_size)
    printf("we got too many colors!\n");

  return extra->_mrc_idx;  
}

img_t *indexed_palette_img_create(img_type_t type, uint16_t w, uint16_t h, uint32_t c)
{
  img_t                   *ret;
  indexed_palette_extra_t *pextra;
  uint16_t                 color_idx;
  uint32_t                 data_size;

  // for this type of image, we have an "extra" structure
  // which we allocate first
  pextra = (indexed_palette_extra_t *) malloc(sizeof(indexed_palette_extra_t));
  
  if(pextra)
  {
    memset((void*)pextra, 0, sizeof(indexed_palette_extra_t));

    pextra->next_color = 0;
    pextra->_mrc_idx = 0;
    pextra->_mrc = 0xFFFFFFFF;

    int padding = 8;

    switch(type)
    {
      case img_type_indexed_palette15:
        pextra->palette_max_size = 15;
        data_size = w * h * sizeof(uint8_t) / 2 + padding;
      break;

      case img_type_indexed_palette255:
        pextra->palette_max_size = 255;
        data_size = w * h * sizeof(uint8_t);
      break;

      case img_type_indexed_palette4095:
        pextra->palette_max_size = 4095;
        // our allocation is for 1.5 bytes per pixel
        data_size = w * h * sizeof(uint8_t);

        // add 50% extra to the buffer, plus some padding in case of odd sizes
        data_size += ((data_size)>>1) + padding;
      break;

      default:
        printf("Erroneous image type passed in to %s line %d\n", __FILE__,__LINE__);
    }

    // Allocate the image structure, with the data buffer
    // at the end
    ret = (img_t*) malloc(sizeof(img_t) +  data_size);
    if(ret)
    {
      ret->img_type = type;
      ret->width = w;
      ret->height = h;
      ret->extra = pextra;
      ret->data_size = data_size;
      indexed_palette_img_fill_vtable(ret);
      
      color_idx = _get_color_idx(ret, c);
      memset(ret->data, color_idx, ret->data_size);
    }
    else
    {
      /* if the image allocation failed, then free the extra
         space, if there was extra space allocated */
      if(pextra)
        free(pextra);
    }
  }

  return ret;
}

void indexed_palette_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    if(pimg->extra)
      free(pimg->extra);
    free(pimg);
  }
}

void indexed_palette_img_plot255(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint8_t color_idx;
  uint32_t pixel_index;
  uint8_t *data = (uint8_t *) pimg->data;

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      color_idx = _get_color_idx(pimg, c);
      pixel_index = y * pimg->width + x;
      data[pixel_index] = color_idx;
    }
  }
}

void indexed_palette_img_plot15(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint8_t color_idx;
  uint8_t existing_value;
  uint32_t pixel_index;
  uint8_t  hi_lo;
  uint8_t *data = (uint8_t *) pimg->data;

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      color_idx = _get_color_idx(pimg, c);
      pixel_index = y * pimg->width + x;
      hi_lo = pixel_index & 1;
      pixel_index >>= 1;
      existing_value = data[pixel_index];

      if(!hi_lo)
      {
        // LO nibble
        data[pixel_index] = (existing_value & 0xF0) | (color_idx & 0x0F);
      }
      else
      {
        // HI nibble
        data[pixel_index] = (existing_value & 0x0F) | ((color_idx<<4) & 0xF0);
      }
    }
  }
}

void indexed_palette_img_plot4095(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  printf("Not implemented yet!\n");
}

void indexed_palette_img_dump_stats(img_t *pimg, const char* title)
{
  int i;
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  if(pimg)
  {
    printf("%s, Width:  %d, ", title, pimg->width);
    printf("Height: %d, ", pimg->height);
    printf("Color count: %d\n", pextra->next_color);
    for(i = 0; i < 255; i+=4)
    {
      printf("% 3d: 0x%06X  0x%06X  0x%06X  0x%06X\n", i, 
          pextra->palette[i], pextra->palette[i+1],pextra->palette[i+2],pextra->palette[i+3]);
    }
  }
}

uint32_t indexed_palette_img_getpixelclamped255 (img_t *pimg, uint16_t x, uint16_t y)
{
  uint8_t color_idx;
  uint8_t *data = (uint8_t*) (pimg->data);
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  color_idx = data[y*pimg->width + x];

  return pextra->palette[color_idx];
}

uint32_t indexed_palette_img_getpixelclamped15 (img_t *pimg, uint16_t x, uint16_t y)
{
  uint8_t  color_idx;
  uint8_t  existing_value;
  uint32_t pixel_index;
  uint8_t  hi_lo;  
  uint8_t *data = (uint8_t*) (pimg->data);
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  pixel_index = y*pimg->width + x;
  hi_lo = pixel_index & 1;
  pixel_index >>= 1;
  existing_value = data[pixel_index];

  if(!hi_lo)
  {
    // LO nibble
    color_idx = existing_value & 0x0F;
  }
  else
  {
    // HI nibble
    color_idx = (existing_value & 0xF0) >> 4;
  }

  return pextra->palette[color_idx];
}

uint32_t indexed_palette_img_getpixelclamped4095 (img_t *pimg, uint16_t x, uint16_t y)
{
  printf("Not implemented!\n");
  return 0;
}



void indexed_palette_img_fill_vtable(img_t *pimg)
{
  switch(pimg->img_type)
  {
    case img_type_indexed_palette15:
      pimg->plot_func = indexed_palette_img_plot15;
      pimg->get_pixel_func = indexed_palette_img_getpixelclamped15;      
    break;

    case img_type_indexed_palette255:
      pimg->plot_func = indexed_palette_img_plot255;    
      pimg->get_pixel_func = indexed_palette_img_getpixelclamped255;
    break;

    case img_type_indexed_palette4095:
      pimg->plot_func = indexed_palette_img_plot4095;
      pimg->get_pixel_func = indexed_palette_img_getpixelclamped4095;      
    break;

    default:
      printf("Erroneous img type passed into %s line %d\n", __FILE__, __LINE__);
  }

  pimg->destroy_func = indexed_palette_img_destroy;
  pimg->dump_stats_func = indexed_palette_img_dump_stats;
}
