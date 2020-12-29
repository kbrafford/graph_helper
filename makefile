all : test.exe

include simple_img_system/makefile

SRC += test.c

test.exe: $(SRC)
	echo $(SRC)
	gcc -o $@ -Isimple_img_system $(SRC) -lm