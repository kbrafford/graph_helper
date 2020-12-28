#ifndef __SIMPLE_IMG_SYSTEM_H__
#define __SIMPLE_IMG_SYSTEM_H__
#include <stdint.h>

#define _CLAMP(v, min, max) if (v < min) { v = min; } else if (v > max) { v = max; } 
#define RGB(r, g, b) ((r)<<16 | ((g)<<8) | (b))

typedef enum _img_type
{
  img_type_indexed_palette255   = 0,
  img_type_indexed_palette15    = 1,
  img_type_indexed_palette4095  = 2,
  img_type_p565                 = 3,
  img_type_p332                 = 4,
  img_type_grayscale8           = 5,
  img_type_grayscale4           = 6,
} img_type_t;

typedef struct _img_point_t
{
  uint16_t x;
  uint16_t y;
} img_point_t;

typedef struct _img_t
{
  img_type_t img_type;
  uint16_t   width;
  uint16_t   height;
  uint16_t   num_channels;
  void*      extra;

  // Implementation functions (the "vtable")
  void     (*plot_func)(struct _img_t*, uint16_t, uint16_t, uint32_t);
  void     (*destroy_func)(struct _img_t*);
  void     (*dump_stats_func)(struct _img_t *pimg, const char *title);

  uint32_t (*get_pixel_func)(struct _img_t *pimg, uint16_t x, uint16_t y);

  // the data buffer for the image will be stored at the end of this driver header
  uint32_t data_size;
  uint8_t  data[0];
} img_t;


img_t *img_create(img_type_t type, uint16_t width, uint16_t height, uint32_t c);

void img_destroy(img_t *pimg);
void img_plot_line(img_t *pimg, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t t, uint32_t c);
void img_plot_line_antialias(img_t *pimg, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t c);
void img_plot_circle(img_t *pimg, uint16_t x0, uint16_t y0, uint16_t radius, uint32_t c);
void img_plot_vline(img_t *pimg, int32_t x, int32_t y0, int32_t y1, uint8_t t, uint32_t c);
void img_plot_hline(img_t *pimg, int32_t x0, int32_t x1, int32_t y, uint8_t t, uint32_t c);
void img_plot_path(img_t *pimg, img_point_t *ppath, uint16_t count, uint8_t t, uint32_t c);
void img_bit_blt(img_t *pdst_img, uint16_t xd, uint16_t yd, 
                 img_t *psrc_img, uint16_t xs, uint16_t ys, uint16_t width, uint16_t height);

img_t *img_resize (img_type_t new_type, img_t *psrc_img, float scale, int degree);
uint32_t img_save_png(img_t *pimg, const char *fname);
void img_dump_stats(img_t *pimg, const char* title);
#endif
