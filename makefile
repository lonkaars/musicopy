CC = gcc
LD = gcc
RM = rm -f
CFLAGS = -lcwalk -linih -lcrypto

OBJECTS := $(patsubst %.c,%.o, *.c)
DESTDIR = /usr/local

all: musicopy

%.o: %.c
	$(CC) -static -c $< $(CFLAGS)

musicopy: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o musicopy

install: musicopy
	mkdir -p $(DESTDIR)/bin
	cp -v musicopy $(DESTDIR)/bin

uninstall:
	rm -v $(DESTDIR)/bin/musicopy

clean:
	$(RM) musicopy

distclean: clean
	$(RM) *.o
