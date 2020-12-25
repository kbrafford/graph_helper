
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS
#include "miniz.h"

#include <math.h>
#include <stdio.h>

#include "simple_img_system.h"
#include "indexed_palette_img.h"
#include "p565_img.h"

img_t *img_create(img_type_t type, uint16_t width, uint16_t height, uint32_t c)
{
  img_t *ret;

  ret = (img_t*) malloc(sizeof(img_t));

  switch(type)
  {
    case img_type_indexed_palette255:
    case img_type_indexed_palette15:
    case img_type_indexed_palette4095:
      ret->img_type = type;
      ret->width = width;
      ret->height = height;
      ret->extra = (void*) indexed_palette_img_create(ret, c);

      indexed_palette_img_fill_vtable(ret);
    break;

    case img_type_p565:
      ret->img_type = type;
      ret->width = width;
      ret->height = height;
      ret->extra = (void*) p565_img_create(ret, c);

      p565_img_fill_vtable(ret);
    break;
  }

  return ret;
}

void img_destroy(img_t *pimg)
{
  if(pimg)
  {
    pimg->destroy_func(pimg);
  }
}


// From: https://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C 

void img_plot_circle(img_t *pimg, uint16_t x0, uint16_t y0, uint16_t radius, uint32_t c)
{
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
 
    pimg->plot_func(pimg, x0, y0 + radius, c);
    pimg->plot_func(pimg, x0, y0 - radius, c);
    pimg->plot_func(pimg, x0 + radius, y0, c);
    pimg->plot_func(pimg, x0 - radius, y0, c);
 
    while(x < y) 
    {
        if(f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;
        pimg->plot_func(pimg, x0 + x, y0 + y, c);
        pimg->plot_func(pimg, x0 - x, y0 + y, c);
        pimg->plot_func(pimg, x0 + x, y0 - y, c);
        pimg->plot_func(pimg, x0 - x, y0 - y, c);
        pimg->plot_func(pimg, x0 + y, y0 + x, c);
        pimg->plot_func(pimg, x0 - y, y0 + x, c);
        pimg->plot_func(pimg, x0 + y, y0 - x, c);
        pimg->plot_func(pimg, x0 - y, y0 - x, c);
    }
}

// From: https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C

static void _img_plot_line(img_t *pimg, int32_t x0, int32_t y0,
                           int32_t x1, int32_t y1, uint32_t c)
{
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
    pimg->plot_func(pimg, x0, y0, c);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}


void img_plot_line(img_t *pimg, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t t, uint32_t c)
{
  int i, j, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(t)
  {
    for(i = start; i < end; i++)
      for(j = start; j < end; j++)
      {
        _img_plot_line(pimg, x0+j, y0+i, x1+j, y1+i, c);
      }
  }
}



void img_plot_vline(img_t *pimg, int32_t x, int32_t y0, int32_t y1, uint8_t t, uint32_t c)
{
  int i, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(pimg)
  {
    for(i = start; i < end; i++)
      img_plot_line(pimg, x+i, y0, x+i, y1, 1, c);
  }
}


void img_plot_hline(img_t *pimg, int32_t x0, int32_t x1, int32_t y, uint8_t t, uint32_t c)
{
  int i, start, end;
  
  start = t&1 ? -(t+1)/2 + 1 : -t/2 + 1;
  end = start + t;

  if(pimg)
  {
    for(i = start; i < end; i++)
      img_plot_line(pimg, x0, y+i, x1, y+i, 1, c);
  }
}


void img_plot_path(img_t *pimg, img_point_t *ppath, uint16_t count, uint8_t t, uint32_t c)
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
        img_plot_line(pimg, x, y, ppath[i].x, ppath[i].y, t, c);
        x = ppath[i].x;
        y = ppath[i].y;
      }
    }
  }

  return;
}



// PNG saving, courtest of miniz: https://github.com/richgel999/miniz

uint32_t img_save_png(img_t *pimg, const char *fname)
{
  uint32_t rc = 1;
  int      y, x;
  uint32_t c;
  uint32_t input_idx = 0;
  uint32_t output_idx = 0;
  size_t   png_data_size = 0;
  FILE     *fp;
  uint32_t len;

  if(pimg)
  {
    len = pimg->width * pimg->height * 3;
    uint8_t *expanded_image_buffer = (uint8_t*) malloc(len);

    pimg->expand_func(pimg, expanded_image_buffer, len);

    void *pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(expanded_image_buffer,
                             pimg->width, pimg->height, 3, &png_data_size, 6, MZ_FALSE);

    fp = fopen(fname, "wb");
    fwrite(pPNG_data, 1, png_data_size, fp);
    fclose(fp);

    rc = 0;
  }

  return rc;
}


void img_dump_stats(img_t *pimg, const char* title)
{
  pimg->dump_stats_func(pimg, title);
}


// FROM: https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/

// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
// when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
// between B and C.  A and B are used to calculate slopes at the edges.
static float _CubicHermite (float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;
 
    return a*t*t*t + b*t*t + c*t + d;
}

static float _Lerp (float A, float B, float t)
{
    return A * (1.0f - t) + B * t;
}



static uint32_t _GetPixelClamped (img_t *pimg, int x, int y)
{
  return pimg->get_pixel_func(pimg, x, y);
}

static uint32_t _SampleNearest (img_t *pimg, float u, float v)
{
    // calculate coordinates
    int xint = (int)(u * pimg->width);
    int yint = (int)(v * pimg->height);
 
    // return pixel
    return _GetPixelClamped(pimg, xint, yint);
}

static uint32_t _SampleLinear (img_t *pimg, float u, float v)
{
  int i;
 
  typedef union _pixel_t
  {
    uint32_t as_uint32;
    uint8_t  as_uint8[4];
  } pixel_t;

  float col0, col1, value;

  // calculate coordinates -> also need to offset by half a pixel to keep image from shifting down and left half a pixel
  float x = (u * pimg->width) - 0.5f;
  int xint = (int)x;
  float xfract = x - floor(x);
 
  float y = (v * pimg->height) - 0.5f;
  int yint = (int)y;
  float yfract = y - floor(y);
 
  // get pixels
  pixel_t p00 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint + 0) };
  pixel_t p10 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint + 0) };
  pixel_t p01 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint + 1) };
  pixel_t p11 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint + 1) };
 
  // interpolate bi-linearly!
  pixel_t ret;
  for (i = 0; i < 3; i++)
  {
    col0  = _Lerp(p00.as_uint8[i], p10.as_uint8[i], xfract);
    col1  = _Lerp(p01.as_uint8[i], p11.as_uint8[i], xfract);
    value = _Lerp(col0, col1, yfract);

    _CLAMP(value, 0.0f, 255.0f);

    ret.as_uint8[i] = (uint8_t)value & 0xF8;
  }

  return ret.as_uint32;
}


static uint32_t _SampleBicubic (img_t *pimg, float u, float v)
{
  typedef union _pixel_t
  {
    uint32_t as_uint32;
    uint8_t  as_uint8[4];
  } pixel_t;

    // calculate coordinates -> also need to offset by half a pixel to keep image from shifting down and left half a pixel
    float x = (u * pimg->width) - 0.5;
    int xint = (int)x;
    float xfract = x - floor(x);
 
    float y = (v * pimg->height) - 0.5;
    int yint = (int)y;
    float yfract = y - floor(y);
 
    // 1st row
    pixel_t p00 = { .as_uint32 = _GetPixelClamped(pimg, xint - 1, yint - 1) };
    pixel_t p10 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint - 1) };
    pixel_t p20 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint - 1) };
    pixel_t p30 = { .as_uint32 = _GetPixelClamped(pimg, xint + 2, yint - 1) };
 
    // 2nd row
    pixel_t p01 = { .as_uint32 = _GetPixelClamped(pimg, xint - 1, yint + 0) };
    pixel_t p11 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint + 0) };
    pixel_t p21 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint + 0) };
    pixel_t p31 = { .as_uint32 = _GetPixelClamped(pimg, xint + 2, yint + 0) };
 
    // 3rd row
    pixel_t p02 = { .as_uint32 = _GetPixelClamped(pimg, xint - 1, yint + 1) };
    pixel_t p12 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint + 1) };
    pixel_t p22 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint + 1) };
    pixel_t p32 = { .as_uint32 = _GetPixelClamped(pimg, xint + 2, yint + 1) };
 
    // 4th row
    pixel_t p03 = { .as_uint32 = _GetPixelClamped(pimg, xint - 1, yint + 2) };
    pixel_t p13 = { .as_uint32 = _GetPixelClamped(pimg, xint + 0, yint + 2) };
    pixel_t p23 = { .as_uint32 = _GetPixelClamped(pimg, xint + 1, yint + 2) };
    pixel_t p33 = { .as_uint32 = _GetPixelClamped(pimg, xint + 2, yint + 2) };
 
    // interpolate bi-cubically!
    // Clamp the values since the curve can put the value below 0 or above 255
    pixel_t ret;
    for (int i = 0; i < 3; ++i)
    {
        float col0 = _CubicHermite(p00.as_uint8[i], p10.as_uint8[i], p20.as_uint8[i], p30.as_uint8[i], xfract);
        float col1 = _CubicHermite(p01.as_uint8[i], p11.as_uint8[i], p21.as_uint8[i], p31.as_uint8[i], xfract);
        float col2 = _CubicHermite(p02.as_uint8[i], p12.as_uint8[i], p22.as_uint8[i], p32.as_uint8[i], xfract);
        float col3 = _CubicHermite(p03.as_uint8[i], p13.as_uint8[i], p23.as_uint8[i], p33.as_uint8[i], xfract);
        float value = _CubicHermite(col0, col1, col2, col3, yfract);
        _CLAMP(value, 0.0f, 255.0f);
        ret.as_uint8[i] = (uint8_t)value & 0xF8;
    }
    return ret.as_uint32;
}


img_t *img_resize (img_type_t new_type, img_t *psrc_img, float scale, int degree)
{
  img_t      *pdst_img;
  int         x,y;
  uint32_t    sample;
  float       u,v;

  uint16_t width  = (uint16_t) (float)(psrc_img->width)*scale;
  uint16_t height = (uint16_t) (float)(psrc_img->height)*scale;

  pdst_img = img_create(new_type, width, height, RGB(0,0,0));

  for (y = 0; y < pdst_img->height; y++)
  {
    v = ((float)y) / (float)(pdst_img->height - 1);

    for (x = 0; x < pdst_img->width; x++)
    {
      u = ((float)x) / (float)(pdst_img->width - 1);
 
      if (degree == 0)
        sample = _SampleNearest(psrc_img, u, v);
      else if (degree == 1)
        sample = _SampleLinear(psrc_img, u, v);
      else if (degree == 2)
        sample = _SampleBicubic(psrc_img, u, v);

      pdst_img->plot_func(pdst_img, x, y, sample & 0x00FFFFFF);
    }
  }

  return pdst_img;
}


// FROM: https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C

inline void _dla_changebrightness(uint32_t from, uint32_t *to, float br)
{
  typedef union _pixel_t
  {
    uint32_t as_uint32;
    uint8_t  as_uint8[4];
  } pixel_t;

  pixel_t pel = { .as_uint32 = from };

  if ( br > 1.0 ) br = 1.0;
  /* linear... Maybe something more complex could give better look */
  pel.as_uint8[2] = br * (float)pel.as_uint8[2];
  pel.as_uint8[1] = br * (float)pel.as_uint8[1];
  pel.as_uint8[0] = br * (float)pel.as_uint8[0];

  *to = pel.as_uint32;
}



inline void _dla_plot(img_t *pimg, int x, int y, uint32_t col, float br)
{
  uint32_t oc;

  _dla_changebrightness(col, &oc, br);

  pimg->plot_func(pimg, x, y, oc);
}


#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
 
#define swap_(a, b) do{ unsigned int tmp; tmp = a; a = b; b = tmp; }while(0)

void img_plot_line_antialias(img_t *pimg, unsigned int x1, unsigned int y1,
                                          unsigned int x2, unsigned int y2, uint32_t c)
{
  double dx = (double)x2 - (double)x1;
  double dy = (double)y2 - (double)y1;
  if ( fabs(dx) > fabs(dy) ) {
    if ( x2 < x1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dy / dx;
    double xend = round_(x1);
    double yend = y1 + gradient*(xend - x1);
    double xgap = rfpart_(x1 + 0.5);
    int xpxl1 = xend;
    int ypxl1 = ipart_(yend);
    
    _dla_plot(pimg, xpxl1, ypxl1, c, rfpart_(yend)*xgap);
    //plot_(xpxl1, ypxl1, rfpart_(yend)*xgap);

    _dla_plot(pimg, xpxl1, ypxl1+1, c, fpart_(yend)*xgap);
    //plot_(xpxl1, ypxl1+1, fpart_(yend)*xgap);

    double intery = yend + gradient;
 
    xend = round_(x2);
    yend = y2 + gradient*(xend - x2);
    xgap = fpart_(x2+0.5);
    int xpxl2 = xend;
    int ypxl2 = ipart_(yend);

    _dla_plot(pimg, xpxl2, ypxl2, c, rfpart_(yend)*xgap);
    //plot_(xpxl2, ypxl2, rfpart_(yend) * xgap);
    _dla_plot(pimg, xpxl2, ypxl2+1, c, fpart_(yend) * xgap);
    //plot_(xpxl2, ypxl2 + 1, fpart_(yend) * xgap);
 
    int x;
    for(x=xpxl1+1; x < xpxl2; x++) {
      _dla_plot(pimg, x, ipart_(intery), c, rfpart_(intery));      
      //plot_(x, ipart_(intery), rfpart_(intery));
      _dla_plot(pimg, x, ipart_(intery) +1, c, fpart_(intery));
      //plot_(x, ipart_(intery) + 1, fpart_(intery));
      intery += gradient;
    }
  } else {
    if ( y2 < y1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dx / dy;
    double yend = round_(y1);
    double xend = x1 + gradient*(yend - y1);
    double ygap = rfpart_(y1 + 0.5);
    int ypxl1 = yend;
    int xpxl1 = ipart_(xend);
    _dla_plot(pimg, xpxl1, ypxl1, c, rfpart_(xend)*ygap);    
    //plot_(xpxl1, ypxl1, rfpart_(xend)*ygap);
    _dla_plot(pimg, xpxl1+1, ypxl1, c, fpart_(xend)*ygap);    
    //plot_(xpxl1 + 1, ypxl1, fpart_(xend)*ygap);
    double interx = xend + gradient;
 
    yend = round_(y2);
    xend = x2 + gradient*(yend - y2);
    ygap = fpart_(y2+0.5);
    int ypxl2 = yend;
    int xpxl2 = ipart_(xend);

    _dla_plot(pimg, xpxl2, ypxl2, c, rfpart_(xend) * ygap);        
    //plot_(xpxl2, ypxl2, rfpart_(xend) * ygap);
    _dla_plot(pimg, xpxl2+1, ypxl2, c, fpart_(xend) * ygap);        
    //plot_(xpxl2 + 1, ypxl2, fpart_(xend) * ygap);
 
    int y;
    for(y=ypxl1+1; y < ypxl2; y++) {
      _dla_plot(pimg, ipart_(interx), y, c, rfpart_(interx));
      //plot_(ipart_(interx), y, rfpart_(interx));
      _dla_plot(pimg, ipart_(interx)+1, y, c, fpart_(interx));
      //plot_(ipart_(interx) + 1, y, fpart_(interx));
      interx += gradient;
    }
  }
}
#undef swap_
#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_

