CC=gcc
CFLAGS=-Wall -Wextra -L. -lftd2xx -lpthread -ldl -g
LIBS=lib/libftd2xx.a

SRC = crux.c crux_parser.c crux_stitch.c
OBJ = $(SRC:.c=.o)

all : crux_api

crux_api: $(OBJ)
	ar -x lib/libftd2xx.a 
	ar -qc libcruxapi.a *.o
	rm *.o
