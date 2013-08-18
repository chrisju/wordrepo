all: ../../bin/tidy ../../bin/gen
../../bin/gen: gen.c common.c common.h
	gcc -g `pkg-config --cflags --libs glib-2.0` -o gen gen.c common.c
	mv gen ../../bin
../../bin/tidy: tidy.c prepare.c common.c common.h
	gcc -g `pkg-config --cflags --libs glib-2.0` -o tidy tidy.c prepare.c common.c
	mv tidy ../../bin

clean:
	-rm -f ../../bin/tidy
	-rm -f ../../bin/gen

