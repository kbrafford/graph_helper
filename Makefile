test.exe: test.c simple_img_system/miniz.c \
          simple_img_system/indexed_palette_img.c simple_img_system/simple_img_system.c simple_img_system/p565_img.c \
          simple_img_system/grayscale_img.c simple_img_system/rgb888_img.c \
					simple_img_system/miniz.h simple_img_system/indexed_palette_img.h simple_img_system/simple_img_system.h simple_img_system/p565_img.h \
					simple_img_system/grayscale_img.h simple_img_system/rgb888_img.h 
	gcc -O3 -Isimple_img_system -o test.exe test.c simple_img_system/simple_img_system.c \
	        simple_img_system/miniz.c simple_img_system/indexed_palette_img.c \
					simple_img_system/p565_img.c simple_img_system/grayscale_img.c \
					simple_img_system/rgb888_img.c

clean:
	erase test.exe