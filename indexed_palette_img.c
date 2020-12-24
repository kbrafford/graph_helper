
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
    //printf("mallocing memory: %d bytes\n", pimg->width * pimg->height * 3); fflush(stdout);
    uint8_t *expanded_image_buffer = (uint8_t*) malloc(pimg->width * pimg->height * 3);

    //printf("filling memory: %d by %d\n", pimg->width , pimg->height); fflush(stdout);

    for(y = 0; y < pimg->height; y++)    
    {
      for(x = 0; x < pimg->width; x++)
      {
        c = pimg->palette[pimg->data[input_idx++]];

        if(firsttime)
        {
          //printf("{Color: 0x%06X, r: %d, g: %d, b: %d}\n", c,
          //        GET_R(c), GET_G(c), GET_B(c)); fflush(stdout);
           
          firsttime = 0;
        }

        expanded_image_buffer[output_idx++] = GET_R(c);
        expanded_image_buffer[output_idx++] = GET_G(c);
        expanded_image_buffer[output_idx++] = GET_B(c);                
      }
      //printf("{out idx: %d}", output_idx); fflush(stdout);
    }

    //printf("Diving into PNG code\n"); fflush(stdout);

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
 
  //printf("p00: 0x%08X, p10: 0x%08X, p01: 0x%08X, p11: 0x%08X\n",
  //  p00.as_uint32, p10.as_uint32, p01.as_uint32, p11.as_uint32);
  //fflush(stdout);


  // interpolate bi-linearly!
  //uint8_t ret[3];
  pixel_t ret;
  for (i = 0; i < 3; i++)
  {
    col0  = _Lerp(p00.as_uint8[i], p10.as_uint8[i], xfract);
    col1  = _Lerp(p01.as_uint8[i], p11.as_uint8[i], xfract);
    value = _Lerp(col0, col1, yfract);

    _CLAMP(value, 0.0f, 255.0f);

    ret.as_uint8[i] = (uint8_t)value & 0xFC;
    //ret[i] = (uint8_t)value;
  }
 
  //printf("ret0: 0x%02X,ret1: 0x%02X,ret2: 0x%02X\n", ret[0], ret[1], ret[2]);
  //fflush(stdout);
  //exit(1);

  //return RGB(ret[2], ret[1], ret[0]);
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
        ret.as_uint8[i] = (uint8_t)value & 0xFC;
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

  //printf("new image:\n");
  //indexed_palette_img_dump_stats(pdst_img);
  //fflush(stdout);

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
