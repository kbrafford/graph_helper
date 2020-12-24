#include "indexed_palette_img.h"
#include <stdio.h>

#define IMG_X 828*3
#define IMG_Y 1792*3

int main()
{
  indexed_palette_img_t *pimg;

  indexed_palette_img_init();

  pimg = indexed_palette_img_create(IMG_X, IMG_Y, RGB(240,240,240));

  indexed_palette_img_plot(pimg, 100, 100, RGB(0,0,255));

  indexed_palette_img_plot_line(pimg, 0,0,IMG_X-1,IMG_Y-1, 1, RGB(255,0,0));
  indexed_palette_img_plot_line(pimg, IMG_X-1,0,0,IMG_Y-1, 1, RGB(255,0,0));

  indexed_palette_img_plot_line(pimg, 200,200, 1280-200,200, 1, RGB(0,0,255));
  indexed_palette_img_plot_line(pimg, 1280-200,200, 1280-200,720-200, 1, RGB(0,0,255));
  indexed_palette_img_plot_line(pimg, 200,720-200, 1280-200,720-200, 1, RGB(0,0,255));
  indexed_palette_img_plot_line(pimg, 200,200, 200,720-200, 1, RGB(0,0,255));

  img_point_t path[] = {
    { 100,100 },
    { 150, 50 },
    { 200,100 },
    { 250, 50 },
  };
  const int pathsize = sizeof(path) / sizeof(path[0]);

  indexed_palette_img_plot_path(pimg, &path[0], pathsize, 2, RGB(0,255,0));

  indexed_palette_img_plot_vline(pimg, 300, 10, 710, 3, RGB(128, 0, 200));

  indexed_palette_img_plot_hline(pimg, 10, 1280, 10 , 3, RGB(128, 0, 255));

  indexed_palette_img_plot_line(pimg, 3, 3, 250, 250, 7, RGB(64,64, 225));

  indexed_palette_img_save_png(pimg, "keith1.png");
  indexed_palette_img_dump_stats(pimg);
  indexed_palette_img_destroy(pimg);

  return 0;
}