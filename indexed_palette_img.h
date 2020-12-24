#ifndef __INDEXED_PALETTE_IMG_H__
#define __INDEXED_PALETTE_IMG_H__
#include <stdio.h>
#include <stdint.h>

#define RGB(r, g, b) ((r)<<16 | ((g)<<8) | (b))

typedef struct _indexed_palette_img_t
{
  uint32_t palette[256];
  uint32_t _mrc;
  uint16_t width;
  uint16_t height;
  uint8_t  *data;
  uint8_t  next_color;
  uint8_t  _mrc_idx;  
} indexed_palette_img_t;

typedef struct _img_point_t
{
  uint16_t x;
  uint16_t y;
} img_point_t;


indexed_palette_img_t *indexed_palette_img_create(uint16_t width, uint16_t height, uint32_t c);
void indexed_palette_img_destroy(indexed_palette_img_t *pimg);

void indexed_palette_img_plot(indexed_palette_img_t *pimg,
                              uint16_t x, uint16_t y, uint32_t c);

void indexed_palette_img_plot_line(indexed_palette_img_t *pimg, int32_t x0, int32_t y0,
                                   int32_t x1, int32_t y1, uint8_t t, uint32_t c);

void indexed_palette_img_plot_line_antialias(indexed_palette_img_t *pimg, unsigned int x1, unsigned int y1,
                                             unsigned int x2, unsigned int y2, uint32_t c);

void indexed_palette_img_plot_circle(indexed_palette_img_t *pimg, 
                                     uint16_t x0, uint16_t y0, uint16_t radius, uint32_t c);

void indexed_palette_img_plot_vline(indexed_palette_img_t *pimg, int32_t x, 
                                    int32_t y0, int32_t y1, uint8_t t, uint32_t c);

void indexed_palette_img_plot_hline(indexed_palette_img_t *pimg, int32_t x0, int32_t x1, 
                                    int32_t y, uint8_t t, uint32_t c);

void indexed_palette_img_plot_path(indexed_palette_img_t *pimg, img_point_t *ppath, 
                                   uint16_t count, uint8_t t, uint32_t c);

indexed_palette_img_t *indexed_palette_img_resize (indexed_palette_img_t *psrc_img, 
                                                    float scale, int degree);

uint32_t indexed_palette_img_save_png(indexed_palette_img_t *pimg, const char *fname);
void indexed_palette_img_dump_stats(indexed_palette_img_t *pimg, const char* title);

#endif
