#include <malloc.h>
#include <stdio.h>

#include "p565_img.h"

static p565_pixel_t _color2p565(uint32_t c)
{
  p565_pixel_t ret;

  ret.b = COLOR2P565_B(c);
  ret.g = COLOR2P565_G(c);
  ret.r = COLOR2P565_R(c);

  return ret;
}

void *p565_img_create(img_t *pimg, uint32_t c)
{
  int          i;
  p565_pixel_t initial_color;

  if(pimg)
  {
    initial_color = _color2p565(c);

    pimg->data = (p565_pixel_t *) malloc(pimg->width * pimg->height * sizeof(p565_pixel_t));

    if(!pimg->data)
    {
      free(pimg);
      pimg = NULL;
    }
    else
    {
      /* pre-fill the image with the background color */
      for(i = 0; i < pimg->width * pimg->height; i++)
        ((p565_pixel_t *)(pimg->data))[i] = initial_color;
    }
  }
  return pimg;
}

void p565_img_destroy(img_t *pimg)
{
  if(pimg)
  {
    if(pimg->data)
      free(pimg->data);
    free(pimg);
  }
}

void p565_img_plot(img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint32_t pixel_index;
  p565_pixel_t our_color;

  our_color = _color2p565(c);

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      pixel_index = y * pimg->width + x;
      ((p565_pixel_t *)pimg->data)[pixel_index] = our_color;
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
  p565_pixel_t *data = (p565_pixel_t*) (pimg->data);
  p565_pixel_t pel;

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  pel = data[y*pimg->width + x];

  return RGB((pel.r <<3), (pel.g << 2), (pel.b << 3));
}

void p565_img_expand(img_t *pimg, uint8_t *buffer, uint32_t max_count)
{
  int x, y;
  p565_pixel_t *data = (p565_pixel_t*)pimg->data;
  p565_pixel_t p;
  uint32_t output_idx = 0;

  for(y = 0; y < pimg->height; y++)    
  {
    for(x = 0; x < pimg->width; x++)
    {
      p = data[pimg->width * y + x];

      buffer[output_idx++] = p.r << 3;
      buffer[output_idx++] = p.g << 2;
      buffer[output_idx++] = p.b << 3;                
    }
  }
}

void p565_img_fill_vtable(img_t *pimg)
{
  pimg->plot_func = p565_img_plot;
  pimg->expand_func = p565_img_expand;
  pimg->destroy_func = p565_img_destroy;
  pimg->dump_stats_func = p565_img_dump_stats;
  pimg->get_pixel_func = p565_img_getpixelclamped;
}
