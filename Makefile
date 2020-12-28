test.exe: test.c miniz.c indexed_palette_img.c simple_img_system.c p565_img.c \
          grayscale_img.c rgb888_img.c
	gcc -O3 -o test.exe test.c miniz.c indexed_palette_img.c simple_img_system.c p565_img.c \
	           grayscale_img.c rgb888_img.c

clean:
	erase test.exe