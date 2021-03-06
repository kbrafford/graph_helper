#include "simple_img_system.h"
#include <stdio.h>

#define IMG_X 1920*2
#define IMG_Y 1080*2

/* PICK WHICH STYLE OF IMAGE YOU WANT */
/* Note that if you use indexed_palette, you will run out of colors fast
   if drawing a lot of different things */

//img_type_t type = img_type_p565;
//img_type_t type = img_type_indexed_palette15;
img_type_t type = img_type_indexed_palette255;
//img_type_t type = img_type_p332;
//img_type_t type = img_type_grayscale8;
//img_type_t type = img_type_rgb888;

int main()
{
  img_t *pimg;
 
  pimg = img_create(type, IMG_X, IMG_Y, RGB(255,255,255));

  img_plot_line(pimg, 0,0,IMG_X-1,IMG_Y-1, 3, RGB(0,0,255));
  img_plot_line(pimg, IMG_X-1,0,0,IMG_Y-1, 3, RGB(0,0,255));
  img_plot_vline(pimg, IMG_X/2-1, 5, IMG_Y-6, 6, RGB(128, 0, 200));

  img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4, RGB(255,0,25));
  img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4-1, RGB(255,0,25));
  img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4+1, RGB(255,0,25));
  img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4-2, RGB(255,0,25));
  img_plot_circle(pimg, IMG_X/2-1, IMG_Y/2-1, IMG_X/4+2, RGB(255,0,25));

  img_plot_line(pimg, 0, IMG_Y/2-10, IMG_X-1, IMG_Y/2+10, 4, RGB(0,0,0));
  img_plot_line(pimg, IMG_X - 10, 5, 10, IMG_Y - 5, 3, RGB(0,0,0));

  img_plot_line(pimg, 200,200, 1280-200,200, 3, RGB(0,0,255));
  img_plot_line(pimg, 1280-200,200, 1280-200,720-200, 3, RGB(0,0,255));
  img_plot_line(pimg, 200,720-200, 1280-200,720-200, 3, RGB(0,0,255));
  img_plot_line(pimg, 200,200, 200,720-200, 3, RGB(0,0,255));

  img_plot_line          (pimg, 50,  900, 1900,400, 3, RGB(255,165,0));


  img_point_t path[] = {
    { 5, 5 },
    { IMG_X-5, 5 },
    { IMG_X-5, IMG_Y-5 },
    { 5, IMG_Y-5 },
    { 5, 5 },    
  };
  const int pathsize = sizeof(path) / sizeof(path[0]);

  img_plot_path(pimg, &path[0], pathsize, 5, RGB(255,255,0));
  img_plot_vline(pimg, 300, 10, 710, 6, RGB(128, 0, 200));
  img_plot_hline(pimg, 10, 1280, 10 , 6, RGB(128, 0, 255));
  img_plot_line(pimg, 3, 3, 250, 250, 7, RGB(64,64, 225));

  img_t *pimg_nn, *pimg_bil, *pimg_bic;
  img_t *pimg_bil2, *pimg_bic2;

  pimg_nn  = img_resize(img_type_rgb888, pimg, 1280, 0, 1, 0);

  pimg_bil  = img_resize(img_type_rgb888, pimg, 1280, 0, 2, 1);
  pimg_bil2 = img_resize(img_type_rgb888, pimg, 1280, 0, 3, 1);

  pimg_bic = img_resize(img_type_rgb888, pimg, 1280, 0, 2, 2);
  pimg_bic2 = img_resize(img_type_rgb888, pimg, 1280, 0, 3, 2);



  img_save_png(pimg, "image.png");
  //img_save_bmp(pimg, "image.bmp");

  img_save_png(pimg_nn, "image_nn.png");
  img_save_png(pimg_bil, "image_bil.png");
  img_save_png(pimg_bil2, "image_bil2.png");  
  img_save_png(pimg_bic, "image_bic.png");
  img_save_png(pimg_bic2, "image_bic2.png");

  img_dump_stats(pimg,     "Original");
  img_dump_stats(pimg_nn,  "Nearest Neighbor");
  img_dump_stats(pimg_bil, "Bilinear");
  img_dump_stats(pimg_bil2, "Bilinear 2");

  img_dump_stats(pimg_bic, "Bicubic");
  img_dump_stats(pimg_bic2, "Bicubic2 2");

  img_destroy(&pimg);
  img_destroy(&pimg_nn);
  img_destroy(&pimg_bil);
  img_destroy(&pimg_bil2);  
  img_destroy(&pimg_bic);
  img_destroy(&pimg_bic2);

  return 0;
}