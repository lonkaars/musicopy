CC = gcc
LD = gcc
RM = rm -f
CFLAGS =

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o, $(SOURCES))

all: musicopy

voerbak: $(OBJECTS)
	$(CC) $(OBJECTS) -o musicopy

clean:
	$(RM) musicopy

distclean: clean
	$(RM) *.o
