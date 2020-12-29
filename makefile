all : test.exe

LFLAGS :=
CFLAGS := -O3

include simple_img_system/makefile

SRC += test.c

test.exe: $(SRC) makefile
	gcc $(CFLAGS) -o $@ -Isimple_img_system $(SRC)$(LFLAGS)