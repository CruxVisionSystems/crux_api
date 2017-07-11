CC=gcc
CFLAGS=-Wall -Wextra -L. -lftd2xx -lpthread -ldl -g
LIBS=ftdi_lib/libftd2xx.a

all: crux_api

crux_api : examples/basic/main.c
	$(CC) examples/basic/main.c crux.c crux_parser.c crux_stitch.c -I. -o bin/crux_output $(LIBS) $(CFLAGS)


