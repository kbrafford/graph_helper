#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "simple_img_system.h"
#include "indexed_palette_img.h"


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

indexed_palette_extra_t *indexed_palette_img_create(img_t *pimg, uint32_t c)
{
  indexed_palette_extra_t *pextra;
  uint8_t                  color_idx;

  pextra = (indexed_palette_extra_t *) malloc(sizeof(indexed_palette_extra_t));

  if(pextra)
  {
    memset((void*)pextra, 0, sizeof(indexed_palette_extra_t));
    pimg->extra = pextra;

    pextra->next_color = 0;
    pextra->_mrc_idx = 0;
    pextra->_mrc = 0xFFFFFFFF;
    
    int buffer_size;
    int padding = 8;

    switch(pimg->img_type)
    {
      case img_type_indexed_palette15:
        pextra->palette_max_size = 15;
        buffer_size = pimg->width * pimg->height * sizeof(uint8_t) / 2 + padding;
        pimg->data = (uint8_t *) malloc(buffer_size);
      break;

      case img_type_indexed_palette255:
        pextra->palette_max_size = 255;
        buffer_size = pimg->width * pimg->height * sizeof(uint8_t);
        pimg->data = (uint8_t *) malloc(buffer_size);
      break;

      case img_type_indexed_palette4095:
        pextra->palette_max_size = 4095;
        // our allocation is for 1.5 bytes per pixel
        buffer_size = pimg->width * pimg->height * sizeof(uint8_t);

        // add 50% extra to the buffer, plus some padding in case of odd sizes
        buffer_size += (buffer_size>>1) + padding;
        pimg->data = (uint8_t *) malloc(buffer_size);
      break;
    }

    if(!pimg->data)
    {
      free(pextra);
      pextra = NULL;
    }
    else
    {
      /* pre-fill the image with the background color */
      /* the background color is always index zero    */
      /* so that works the same in all color palettes */
      color_idx = _get_color_idx(pimg, c);
      memset(pimg->data, color_idx, buffer_size);
    }
  }

  return pextra;
}

void indexed_palette_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    if(pimg->data)
      free(pimg->data);
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


void indexed_palette_img_expand(img_t *pimg, uint8_t *buffer, uint32_t max_count)
{
  int x, y;
  uint32_t c;
  uint32_t output_idx = 0;
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  for(y = 0; y < pimg->height; y++)    
  {
    if(output_idx >= (max_count - 3))
    {
      printf("Not enough room to store raw image for PNG conversion\n");
      break;
    }

    for(x = 0; x < pimg->width; x++)
    {
      c = pimg->get_pixel_func(pimg, x, y);

      if(output_idx >= (max_count - 3))
      {
        printf("Not enough room to store raw image for PNG conversion\n");
        break;
      }

      buffer[output_idx++] = GET_R(c);
      buffer[output_idx++] = GET_G(c);
      buffer[output_idx++] = GET_B(c);
    }
  }
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
  }

  pimg->destroy_func = indexed_palette_img_destroy;
  pimg->dump_stats_func = indexed_palette_img_dump_stats;
  
}
