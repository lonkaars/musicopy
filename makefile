CC = gcc
LD = gcc
RM = rm -f
CFLAGS = -lcwalk -linih

OBJECTS := $(patsubst %.c,%.o, *.c)

all: musicopy

%.o: %.c
	$(CC) -static -c $< $(CFLAGS)

musicopy: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o musicopy

clean:
	$(RM) musicopy

distclean: clean
	$(RM) *.o
