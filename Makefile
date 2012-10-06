CC=gcc
CFLAGS=-Wall -g -O2 -std=c99
LIBS=`pkg-config --libs gtk+-2.0 glib-2.0` -lm -lround
INCS=`pkg-config --cflags gtk+-2.0 glib-2.0`

vatmageddon: vatmageddon.c
	$(CC) $(CFLAGS) -o vatmageddon vatmageddon.c ${INCS} ${LIBS}
	gzip -c vatmageddon.1 > vatmageddon.1.gz

clean:
	rm -f vatmageddon vatmageddon.1.gz
