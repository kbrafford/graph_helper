#include "indexed_palette_img.h"
#include <stdio.h>

#define IMG_X 1920*3
#define IMG_Y 1080*3

int main()
{
  indexed_palette_img_t *pimg;

  pimg = indexed_palette_img_create(IMG_X, IMG_Y, RGB(255,255,255));

  indexed_palette_img_plot_line(pimg, 0,0,IMG_X-1,IMG_Y-1, 2, RGB(0,0,255));
  indexed_palette_img_plot_line(pimg, IMG_X-1,0,0,IMG_Y-1, 2, RGB(0,0,255));
  indexed_palette_img_plot_vline(pimg, IMG_X/2-1, 5, IMG_Y-6, 6, RGB(128, 0, 200));

  indexed_palette_img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4, RGB(255,0,25));
  indexed_palette_img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4-1, RGB(255,0,25));
  indexed_palette_img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4+1, RGB(255,0,25));
  indexed_palette_img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4-2, RGB(255,0,25));
  indexed_palette_img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4+2, RGB(255,0,25));

  indexed_palette_img_plot_line(pimg, 25, 5, IMG_X-26, IMG_Y-5, 2, RGB(0,0,0));
  //indexed_palette_img_plot_line_antialias(pimg, 10, 5, IMG_X - 10, IMG_Y - 5, RGB(0,0,0));

  //indexed_palette_img_plot_line(pimg, 200,200, 1280-200,200, 1, RGB(0,0,255));
  //indexed_palette_img_plot_line(pimg, 1280-200,200, 1280-200,720-200, 1, RGB(0,0,255));
  //indexed_palette_img_plot_line(pimg, 200,720-200, 1280-200,720-200, 1, RGB(0,0,255));
  //indexed_palette_img_plot_line(pimg, 200,200, 200,720-200, 1, RGB(0,0,255));

  img_point_t path[] = {
    { 5, 5 },
    { IMG_X-5, 5 },
    { IMG_X-5, IMG_Y-5 },
    { 5, IMG_Y-5 },
    { 5, 5 },    
  };
  const int pathsize = sizeof(path) / sizeof(path[0]);

  indexed_palette_img_plot_path(pimg, &path[0], pathsize, 5, RGB(255,255,0));

  //indexed_palette_img_plot_vline(pimg, 300, 10, 710, 3, RGB(128, 0, 200));

  //indexed_palette_img_plot_hline(pimg, 10, 1280, 10 , 3, RGB(128, 0, 255));

  //indexed_palette_img_plot_line(pimg, 3, 3, 250, 250, 7, RGB(64,64, 225));

  indexed_palette_img_t *pimg_nn, *pimg_bil, *pimg_bic;

  pimg_nn  = indexed_palette_img_resize(pimg, .25, 0);
  pimg_bil = indexed_palette_img_resize(pimg, .25, 1);
  pimg_bic = indexed_palette_img_resize(pimg, .25, 2);

  indexed_palette_img_save_png(pimg, "image.png");
  indexed_palette_img_save_png(pimg_nn, "image_nn.png");
  indexed_palette_img_save_png(pimg_bil, "image_bil.png");
  indexed_palette_img_save_png(pimg_bic, "image_bic.png");

  indexed_palette_img_dump_stats(pimg, "Original");
  indexed_palette_img_dump_stats(pimg_nn, "Nearest Neighbor");
  indexed_palette_img_dump_stats(pimg_bil, "Bilinear");
  indexed_palette_img_dump_stats(pimg_bic, "Bicubic");

  indexed_palette_img_destroy(pimg);
  indexed_palette_img_destroy(pimg_nn);
  indexed_palette_img_destroy(pimg_bil);
  indexed_palette_img_destroy(pimg_bic);

  return 0;
}