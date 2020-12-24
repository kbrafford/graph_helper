
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS

#include <string.h>
#include <math.h>

#include "miniz.h"
#include "indexed_palette_img.h"

uint8_t _get_color_idx(indexed_palette_img_t *pimg, uint32_t c)
{
  int     i;
  int     found = 0;

  c &= 0xFFFFFF;

  if(c != pimg->_mrc)
  {
    /* see if this is a color we've already seen */
    for(i = 0; i < pimg->next_color; i++)
    {
      if (c == pimg->palette[i])
      {
        pimg->_mrc_idx = i;
        pimg->_mrc = c;
        found = 1;
        break;
      }
    }

    /* see if we need to make a new color */
    if((!found)&&(pimg->next_color < 255))
    {
      pimg->palette[pimg->next_color] = c;
      pimg->_mrc = c;
      pimg->_mrc_idx = pimg->next_color;
      pimg->next_color++;
    }
  }

  if(pimg->next_color == 255)
    printf("we got too many colors!\n");

  return pimg->_mrc_idx;  
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

    pimg->_mrc_idx = 0;
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
      //printf("Color: 0x%06X, Color Index: %d\n", c, color_idx);
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


 
// From: https://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C 

void indexed_palette_img_plot_circle(indexed_palette_img_t *pimg, 
                                     uint16_t x0, uint16_t y0, uint16_t radius, uint32_t c)
{
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
 
    indexed_palette_img_plot(pimg, x0, y0 + radius, c);
    indexed_palette_img_plot(pimg, x0, y0 - radius, c);
    indexed_palette_img_plot(pimg, x0 + radius, y0, c);
    indexed_palette_img_plot(pimg, x0 - radius, y0, c);
 
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
        indexed_palette_img_plot(pimg, x0 + x, y0 + y, c);
        indexed_palette_img_plot(pimg, x0 - x, y0 + y, c);
        indexed_palette_img_plot(pimg, x0 + x, y0 - y, c);
        indexed_palette_img_plot(pimg, x0 - x, y0 - y, c);
        indexed_palette_img_plot(pimg, x0 + y, y0 + x, c);
        indexed_palette_img_plot(pimg, x0 - y, y0 + x, c);
        indexed_palette_img_plot(pimg, x0 + y, y0 - x, c);
        indexed_palette_img_plot(pimg, x0 - y, y0 - x, c);
    }
}

// From: https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C

static void _indexed_palette_img_plot_line(indexed_palette_img_t *pimg, int32_t x0, int32_t y0,
                                           int32_t x1, int32_t y1, uint32_t c)
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



// PNG saving, courtest of miniz: https://github.com/richgel999/miniz

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

  if(pimg)
  {
    uint8_t *expanded_image_buffer = (uint8_t*) malloc(pimg->width * pimg->height * 3);

    for(y = 0; y < pimg->height; y++)    
    {
      for(x = 0; x < pimg->width; x++)
      {
        c = pimg->palette[pimg->data[input_idx++]];

        expanded_image_buffer[output_idx++] = GET_R(c);
        expanded_image_buffer[output_idx++] = GET_G(c);
        expanded_image_buffer[output_idx++] = GET_B(c);                
      }
    }

    void *pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(expanded_image_buffer,
                             pimg->width, pimg->height, 3, &png_data_size, 6, MZ_FALSE);

    fp = fopen(fname, "wb");
    fwrite(pPNG_data, 1, png_data_size, fp);
    fclose(fp);
        
    rc = 0;
  }

  return rc;
}

void indexed_palette_img_dump_stats(indexed_palette_img_t *pimg, const char* title)
{
  int i;

  if(pimg)
  {
    printf("%s, Width:  %d, ", title, pimg->width);
    printf("Height: %d, ", pimg->height);
    printf("Color count: %d\n", pimg->next_color);
    for(i = 0; i < 255; i+=4)
    {
      printf("% 3d: 0x%06X  0x%06X  0x%06X  0x%06X\n", i, 
          pimg->palette[i], pimg->palette[i+1],pimg->palette[i+2],pimg->palette[i+3]);
    }
  }
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

#define _CLAMP(v, min, max) if (v < min) { v = min; } else if (v > max) { v = max; } 

static uint32_t _GetPixelClamped (indexed_palette_img_t *pimg, int x, int y)
{
  uint8_t color_idx;

  _CLAMP(x, 0, pimg->width - 1);
  _CLAMP(y, 0, pimg->height - 1);    

  color_idx = pimg->data[y*pimg->width + x];

  return pimg->palette[color_idx];
}

static uint32_t _SampleNearest (indexed_palette_img_t *pimg, float u, float v)
{
    // calculate coordinates
    int xint = (int)(u * pimg->width);
    int yint = (int)(v * pimg->height);
 
    // return pixel
    return _GetPixelClamped(pimg, xint, yint);
}

static uint32_t _SampleLinear (indexed_palette_img_t *pimg, float u, float v)
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


static uint32_t _SampleBicubic (indexed_palette_img_t *pimg, float u, float v)
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


indexed_palette_img_t *indexed_palette_img_resize (indexed_palette_img_t *psrc_img, float scale, int degree)
{
  indexed_palette_img_t *pdst_img;
  int                    x,y;
  uint32_t               sample;
  float                  u,v;

  /* first, make the new image. We'll copy the background color from the
     source image, even though it should be overwritten by pixel plots.
     And we assume that the zeroth color in the image palette is also the
     background color */
  uint16_t width  = (uint16_t) (float)(psrc_img->width)*scale;
  uint16_t height = (uint16_t) (float)(psrc_img->height)*scale;

  pdst_img = indexed_palette_img_create(width, height, psrc_img->palette[0]);

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

      indexed_palette_img_plot(pdst_img, x, y, sample & 0x00FFFFFF);
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
 
#define plot_(X,Y,D) do{ rgb_color f_;				\
  f_.red = r; f_.green = g; f_.blue = b;			\
  _dla_plot(img, (X), (Y), &f_, (D)) ; }while(0)
 
inline void _dla_plot(indexed_palette_img_t *pimg, int x, int y, uint32_t col, float br)
{
  uint32_t oc;

  _dla_changebrightness(col, &oc, br);

  indexed_palette_img_plot(pimg, x, y, oc);
}
 
#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
 
#define swap_(a, b) do{ unsigned int tmp; tmp = a; a = b; b = tmp; }while(0)

void indexed_palette_img_plot_line_antialias(indexed_palette_img_t *pimg,
                                             unsigned int x1, unsigned int y1,
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