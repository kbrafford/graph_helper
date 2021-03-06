
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS
#include "miniz.h"

#include <math.h>
#include <stdio.h>

#include "simple_img_system.h"
#include "indexed_palette_img.h"
#include "p565_img.h"
#include "grayscale_img.h"
#include "rgb888_img.h"

#if SIS_NUM_THREADS > 1
#include <pthread.h>
#endif


img_t *img_create(img_type_t type, uint16_t width, uint16_t height, uint32_t c)
{
  img_t   *ret = (img_t*)NULL;
  void    *pextra;
  uint32_t data_size;

  switch(type)
  {
    case img_type_indexed_palette255:
    case img_type_indexed_palette15:
    case img_type_indexed_palette4095:
      ret = indexed_palette_img_create(type, width, height, c);
    break;

    case img_type_p332:
    case img_type_p565:
      ret = p565_img_create(type, width, height, c);
    break;

    case img_type_grayscale8:
    case img_type_grayscale4:
      ret = grayscale_img_create(type, width, height, c);
    break;

    case img_type_rgb888:
      ret = rgb888_img_create(type, width, height, c);
    break;
  }
  
  return ret;
}

void img_destroy(img_t **pimg)
{
  if(*pimg)
  {
    (*pimg)->destroy_func(*pimg);
  }

  *pimg = (img_t*)NULL;
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

static void _img_expand(img_t *pimg, uint8_t *buffer, uint32_t max_count)
{
  int x, y;
  uint32_t c;
  uint32_t output_idx = 0;

  for(y = 0; y < pimg->height; y++)    
  {
    if(output_idx > (max_count - pimg->num_channels))
    {
      printf("Not enough room to store raw image for PNG conversion\n");
      break;
    }

    for(x = 0; x < pimg->width; x++)
    {
      c = pimg->get_pixel_func(pimg, x, y);

      if(output_idx > (max_count - pimg->num_channels))
      {
        printf("Not enough room to store raw image for PNG conversion\n");
        break;
      }

      switch(pimg->num_channels)
      {
        case 3:
          buffer[output_idx++] = GET_R(c);
          buffer[output_idx++] = GET_G(c);
        case 1:
          buffer[output_idx++] = GET_B(c);
        break;

        default:
          printf("invalid channel spec in %s line %d\n", __FILE__, __LINE__);
          return;
      }
    }
  }
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
    if((pimg->num_channels != 1) && (pimg->num_channels != 3))
    {
      printf("Error: only 1 or 3 channels supported\n");
      return rc;
    }

    len = pimg->width * pimg->height * pimg->num_channels;
    uint8_t *expanded_image_buffer = (uint8_t*) malloc(len);

    _img_expand(pimg, expanded_image_buffer, len);

    void *pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(expanded_image_buffer,
                             pimg->width, pimg->height, pimg->num_channels,
                             &png_data_size, 6, MZ_FALSE);

    fp = fopen(fname, "wb");
    fwrite(pPNG_data, 1, png_data_size, fp);
    fclose(fp);

    if(expanded_image_buffer)
      free(expanded_image_buffer);

    rc = 0;
  }

  return rc;
}


#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
  uint16_t  bfType;
  uint32_t  bfSize;
  uint16_t  bfReserved1;
  uint16_t  bfReserved2;
  uint32_t  bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t  biPlanes;
  uint16_t  biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

void img_save_bmp (img_t *pimg, const char *fileName)
{
  int y;

  // open the file if we can
  FILE *file;
  file = fopen(fileName, "wb");
    
  if (!file)
    return;

  // make the header info
  BITMAPFILEHEADER header;
  BITMAPINFOHEADER infoHeader;
 
  header.bfType = 0x4D42;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = 54;
 
  infoHeader.biSize = 40;
  infoHeader.biWidth = pimg->width;
  infoHeader.biHeight = pimg->height;
  infoHeader.biPlanes = 1;
  infoHeader.biBitCount = pimg->num_channels * 8;
  infoHeader.biCompression = 0;
  infoHeader.biSizeImage = pimg->width * pimg->height * pimg->num_channels;
  infoHeader.biXPelsPerMeter = 0;
  infoHeader.biYPelsPerMeter = 0;
  infoHeader.biClrUsed = 0;
  infoHeader.biClrImportant = 0;
 
  header.bfSize = infoHeader.biSizeImage + header.bfOffBits;
 
  // write the data and close the file
  fwrite(&header, sizeof(header), 1, file);
  fwrite(&infoHeader, sizeof(infoHeader), 1, file);
    
  /* create a _width_ x 1 RGB image that we can bitblt into */
  /* that way we can use the rgb888 image type as a way   */
  /* to convert from any pixel type into the native BMP   */
  /* format */
  img_t *p_tmp = img_create(img_type_rgb888, pimg->width, 1, 0);

  if(p_tmp)
  {
    for(y = pimg->height; y > 0; y--)
    {
      // copy one raster of the source data into the rgb888 buffer
      img_bit_blt(p_tmp, 0, 0, pimg, 0, y-1, pimg->width, 1);

      // write it to disk
      fwrite(p_tmp->data, 3 * pimg->width, 1, file);
    }

    fclose(file);

    img_destroy(&p_tmp);
  }

  return;
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

    ret.as_uint8[i] = (uint8_t)value;
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
        ret.as_uint8[i] = (uint8_t)value;
    }
    return ret.as_uint32;
}

typedef struct _worker_thread_t
{
  img_t    *pdst_img;
  img_t    *psrc_img;
  int       starting_y;
  int       upper_limit_y;
  int       degree;
} worker_thread_t;

static void *_img_resize_worker_thread(void *arg)
{
  worker_thread_t *my_task = (worker_thread_t*) arg;
  int      x, y;
  float    u,v;
  uint32_t sample;

  for (y = my_task->starting_y; y < my_task->upper_limit_y; y++)
  {
    v = ((float)y) / (float)(my_task->pdst_img->height - 1);

    for (x = 0; x < my_task->pdst_img->width; x++)
    {
      u = ((float)x) / (float)(my_task->pdst_img->width - 1);
  
      if (my_task->degree == 0)
        sample = _SampleNearest(my_task->psrc_img, u, v);
      else if (my_task->degree == 1)
        sample = _SampleLinear(my_task->psrc_img, u, v);
      else if (my_task->degree == 2)
        sample = _SampleBicubic(my_task->psrc_img, u, v);

      my_task->pdst_img->plot_func(my_task->pdst_img, x, y, sample & 0x00FFFFFF);
    }
  }

  return (void*)NULL;
} 

img_t *img_resize (img_type_t new_type, img_t *psrc_img, uint16_t target_w, uint16_t target_h,
                   int steps, int degree)
{
  img_t      *pdst_img, *p_tmp;
  int         x,y;
  uint32_t    sample;
  float       u,v, wscale, hscale;
  int         i;
  uint16_t    width, height;

  /* calculate the scale based on the target width and height. Target height of 0
     means use the same scale is for the width (i.e. maintain aspect ratio) */
  wscale = (float)target_w / psrc_img->width;
  if(!target_h)
  {
    hscale = wscale;
    target_h = (uint16_t) (psrc_img->height * hscale);
  }
  else
    hscale = (float)target_h / psrc_img->height;

  if(!steps)
  {
    wscale = hscale = 1.0f;
    steps = 1;
  }

  /* adjust the scale so we get to the final scale in steps */
  wscale = powf(wscale, (1.0f/steps));
  hscale = powf(hscale, (1.0f/steps));

  for(i = 0; i < steps; i++)
  {
    /* if we are on the last iteration, force the width and height to be precise */
    if(i == steps-1)
    {
      width = target_w;
      height = target_h;
    }
    else
    {
      width  = (uint16_t) (float)(psrc_img->width)*wscale;
      height = (uint16_t) (float)(psrc_img->height)*hscale;
    }

    pdst_img = img_create(new_type, width, height, RGB(0,0,0));

#if SIS_NUM_THREADS == 0 || SIS_NUM_THREADS == 1
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
#else
    pthread_t thread[SIS_NUM_THREADS];
    worker_thread_t tinfo[SIS_NUM_THREADS];

    for(y = 0; y < SIS_NUM_THREADS; y++)
    {
      tinfo[y].degree = degree;
      tinfo[y].pdst_img = pdst_img;
      tinfo[y].psrc_img = psrc_img;
      tinfo[y].starting_y = y * pdst_img->height / SIS_NUM_THREADS;
      tinfo[y].upper_limit_y = (y + 1) * pdst_img->height / SIS_NUM_THREADS;
    }

    /* make sure the last one's limit was exactly the height and wasn't
       messed up by rounding */
    tinfo[SIS_NUM_THREADS-1].upper_limit_y = pdst_img->height;

    /* kick them off */
    for(y = 0; y < SIS_NUM_THREADS; y++)
    {
      pthread_create(&thread[y], NULL, _img_resize_worker_thread, (void*)&(tinfo[y]));
    }

    /* wait for them to complete */
    for(y = 0; y < SIS_NUM_THREADS; y++)
    {
      pthread_join(thread[y], NULL);
    }
#endif

    /* for the next go-round, the source image will be the
       destination image we just cretaed */
    p_tmp = psrc_img;
    psrc_img = pdst_img;

    /* for the first iteration we leave the source image intact,
       since that's the one created by the caller. For each other
       iteration we need to destroy the one we created in the 
       previous iteration after we are done with it. We return
       the last one we created */
    if(i)
    {
      img_destroy(&p_tmp);
    }
  }

  return pdst_img;
}


void img_bit_blt(img_t *pdst_img, uint16_t xd, uint16_t yd, 
                 img_t *psrc_img, uint16_t xs, uint16_t ys, uint16_t width, uint16_t height)
{
  int x, y;
  uint32_t c;

  for(y = 0; y < height; y++)
  {
    for(x = 0; x < width; x++)
    {
      c = _GetPixelClamped(psrc_img, xs+x, ys+y);
      pdst_img->plot_func(pdst_img, xd+x, yd+y, c);
    }
  }
}
