
//#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS

#include "afibalert_if_miniz.h"
#include "indexed_palette_img.h"
#include <string.h>

uint8_t _get_color_idx(indexed_palette_img_t *pimg, uint32_t c)
{
  int     i;

  if(c != pimg->_mrc)
  {
    /* see if this is a color we've already seen */
    for(i = 0; i < pimg->next_color; i++)
    {
      if (c == pimg->palette[i])
      {
        pimg->_mrc_idx = i;
        pimg->_mrc = c;
        break;
      }
    }

    /* see if we need to make a new color */
    if(i == pimg->next_color)
    {
      pimg->palette[pimg->next_color++] = c;
      pimg->_mrc = c;
      pimg->_mrc_idx = i;
    }
  }

  return pimg->_mrc_idx;  
}

void indexed_palette_img_init()
{
}

indexed_palette_img_t *indexed_palette_img_create(uint16_t width, uint16_t height, uint32_t c)
{
  indexed_palette_img_t *pimg;
  uint8_t                color_idx;

  pimg = (indexed_palette_img_t *) malloc(sizeof(indexed_palette_img_t));

  if(pimg)
  {
    memset((void*)pimg, 0, sizeof(indexed_palette_img_t));
    pimg->next_color = 0;
    pimg->width = width;
    pimg->height = height;

    pimg->_mrc_idx = 0xFF;
    pimg->_mrc = 0xFFFFFFFF;

    pimg->data = (uint8_t *) malloc(width * height * sizeof(uint8_t));

    if(!pimg->data)
    {
      free(pimg);
      pimg = NULL;
    }
    else
    {
      /* pre-fill the image with the background color */
      color_idx = _get_color_idx(pimg, c);
      printf("Color: 0x%06X, Color Index: %d\n", c, color_idx);
      memset(pimg->data, color_idx, width * height * sizeof(uint8_t));
    }
  }

  return pimg;
}

void indexed_palette_img_destroy(indexed_palette_img_t *pimg)
{
  if(pimg)
  {
    if(pimg->data)
      free(pimg->data);
    free(pimg);
  }
}

void indexed_palette_img_plot(indexed_palette_img_t *pimg, uint16_t x, uint16_t y, uint32_t c)
{
  uint8_t color_idx;
  uint32_t pixel_index;

  if(pimg)
  {
    if ((x < pimg->width) && (y < pimg->height))
    {
      color_idx = _get_color_idx(pimg, c);
      pixel_index = y * pimg->width + x;
      pimg->data[pixel_index] = color_idx;
    }
  }
}


static void _indexed_palette_img_plot_line(indexed_palette_img_t *pimg, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t c)
{
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
    indexed_palette_img_plot(pimg, x0, y0, c);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}


void indexed_palette_img_plot_line(indexed_palette_img_t *pimg, int32_t x0, int32_t y0, int32_t x1, int32_t y1, 
                                         uint8_t t, uint32_t c)
{
  int i, j, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(t)
  {
    for(i = start; i < end; i++)
      for(j = start; j < end; j++)
      {
        _indexed_palette_img_plot_line(pimg, x0+j, y0+i, x1+j, y1+i, c);
      }
  }
}



void indexed_palette_img_plot_vline(indexed_palette_img_t *pimg, int32_t x, 
                                    int32_t y0, int32_t y1, uint8_t t, uint32_t c)
{
  int i, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(pimg)
  {
    for(i = start; i < end; i++)
      indexed_palette_img_plot_line(pimg, x+i, y0, x+i, y1, 1, c);
  }
}


void indexed_palette_img_plot_hline(indexed_palette_img_t *pimg, int32_t x0, int32_t x1, 
                                    int32_t y, uint8_t t, uint32_t c)
{
  int i, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(pimg)
  {
    for(i = start; i < end; i++)
      indexed_palette_img_plot_line(pimg, x0, y+i, x1, y+i, 1, c);
  }
}



void indexed_palette_img_plot_path(indexed_palette_img_t *pimg, img_point_t *ppath, 
                                   uint16_t count, uint8_t t, uint32_t c)
{
  int i;
  uint16_t x, y;

  if(pimg)
  {
    if(count)
    {
      x = ppath[0].x;
      y = ppath[0].y;

      for(i = 1; i < count; i++)
      {
        indexed_palette_img_plot_line(pimg, x, y, ppath[i].x, ppath[i].y, t, c);
        x = ppath[i].x;
        y = ppath[i].y;
      }
    }
  }

  return;
}





#define GET_R(c) (((c) & 0xFF0000) >> 16)
#define GET_G(c) (((c) & 0x00FF00) >> 8)
#define GET_B(c) ((c) & 0xFF)

uint32_t indexed_palette_img_save_png(indexed_palette_img_t *pimg, const char *fname)
{
  uint32_t rc = 1;
  int      y, x;
  uint32_t c;
  uint32_t input_idx = 0;
  uint32_t output_idx = 0;
  size_t   png_data_size = 0;
  FILE     *fp;
  static int firsttime = 1;

  if(pimg)
  {
    printf("mallocing memory: %d bytes\n", pimg->width * pimg->height * 3); fflush(stdout);
    uint8_t *expanded_image_buffer = (uint8_t*) malloc(pimg->width * pimg->height * 3);

    printf("filling memory: %d by %d\n", pimg->width , pimg->height); fflush(stdout);

    for(y = 0; y < pimg->height; y++)    
    {
      for(x = 0; x < pimg->width; x++)
      {
        c = pimg->palette[pimg->data[input_idx++]];

        if(firsttime)
        {
          printf("{Color: 0x%06X, r: %d, g: %d, b: %d}\n", c,
                  GET_R(c), GET_G(c), GET_B(c)); fflush(stdout);
           
          firsttime = 0;
        }

        expanded_image_buffer[output_idx++] = GET_R(c);
        expanded_image_buffer[output_idx++] = GET_G(c);
        expanded_image_buffer[output_idx++] = GET_B(c);                
      }
      //printf("{out idx: %d}", output_idx); fflush(stdout);
    }

    printf("Diving into PNG code\n"); fflush(stdout);

    void *pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(expanded_image_buffer,
                             pimg->width, pimg->height, 3, &png_data_size, 6, MZ_FALSE);

    fp = fopen(fname, "wb");
    fwrite(pPNG_data, 1, png_data_size, fp);
    fclose(fp);
        
    rc = 0;
  }

  return rc;
}

void indexed_palette_img_dump_stats(indexed_palette_img_t *pimg)
{
  int i;

  if(pimg)
  {
    printf("Width:  %d\n", pimg->width);
    printf("Height: %d\n", pimg->height);
    printf("Color count: %d\n", pimg->next_color);
    for(i = 0; i < 255; i++)
    {
      printf("% 3d: 0x%06X  0x%06X  0x%06X  0x%06X\n", i, 
          pimg->palette[i], pimg->palette[i+1],pimg->palette[i+2],pimg->palette[i+3]);
      i += 4;
    }
  }
}
