all: multipix.c
	gcc $(shell pkg-config --cflags --libs x11 gl) multipix.c -o multipix
