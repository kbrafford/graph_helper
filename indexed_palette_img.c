#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "simple_img_system.h"
#include "indexed_palette_img.h"

uint8_t _get_color_idx(img_t *pimg, uint32_t c)
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
    if((!found)&&(extra->next_color < 255))
    {
      extra->palette[extra->next_color] = c;
      extra->_mrc = c;
      extra->_mrc_idx = extra->next_color;
      extra->next_color++;
    }
  }


  if(extra->next_color == 255)
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

    pimg->data = (uint8_t *) malloc(pimg->width * pimg->height * sizeof(uint8_t));

    if(!pimg->data)
    {
      free(pextra);
      pextra = NULL;
    }
    else
    {
      /* pre-fill the image with the background color */
      color_idx = _get_color_idx(pimg, c);
      memset(pimg->data, color_idx, pimg->width * pimg->height * sizeof(uint8_t));
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

void indexed_palette_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
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

void indexed_palette_img_expand(img_t *pimg, uint8_t *buffer, uint32_t max_count)
{
  int x, y;
  uint8_t *data = (uint8_t*)pimg->data;
  uint32_t c;
  uint32_t input_idx = 0;
  uint32_t output_idx = 0;
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  for(y = 0; y < pimg->height; y++)    
  {
    for(x = 0; x < pimg->width; x++)
    {
      c = pextra->palette[data[input_idx++]];

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

uint32_t indexed_palette_img_getpixelclamped (img_t *pimg, uint16_t x, uint16_t y)
{
  uint8_t color_idx;
  uint8_t *data = (uint8_t*) (pimg->data);
  indexed_palette_extra_t *pextra = (indexed_palette_extra_t*)(pimg->extra);

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  color_idx = data[y*pimg->width + x];

  return pextra->palette[color_idx];
}


void indexed_palette_img_fill_vtable(img_t *pimg)
{
  pimg->plot_func = indexed_palette_img_plot;
  pimg->expand_func = indexed_palette_img_expand;
  pimg->destroy_func = indexed_palette_img_destroy;
  pimg->dump_stats_func = indexed_palette_img_dump_stats;
  pimg->get_pixel_func = indexed_palette_img_getpixelclamped;
}
