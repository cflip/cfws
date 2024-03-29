CC=gcc
LD=gcc

CFLAGS=-Wall -Wextra -pedantic -g
LDFLAGS=

OBJS=cfws.o file.o http.o net.o

DESTDIR=/usr/local/bin/

.PHONY: all clean install

all: cfws

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

cfws: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) cfws

install: all
	install -m 0755 ./cfws -t $(DESTDIR)
