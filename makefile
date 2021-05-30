CC = gcc
LD = gcc
RM = rm -f
CFLAGS = -lcwalk

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o, $(SOURCES))

all: musicopy

musicopy: $(OBJECTS)
	$(CC) -static $(OBJECTS) $(CFLAGS) -o musicopy

clean:
	$(RM) musicopy

distclean: clean
	$(RM) *.o
