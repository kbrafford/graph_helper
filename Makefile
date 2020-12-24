test.exe: test.c miniz.c indexed_palette_img.c
	gcc -O3 -o test.exe test.c miniz.c indexed_palette_img.c

clean:
	erase test.exe