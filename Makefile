CC=gcc
CFLAGS=-Wall -Wextra -L. -lftd2xx -lpthread -ldl
LIBS=libftd2xx.a

all: crux_api

crux_api : crux.c
	$(CC) crux.c crux_parser.c crux_stitch.c -o crux_output $(LIBS) $(CFLAGS)


