vatmageddon: vatmageddon.c
	gcc -Wall -O2 vatmageddon.c -o vatmageddon `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` -lround -lm
	gzip -c vatmageddon.1 > vatmageddon.1.gz

clean:
	rm vatmageddon vatmageddon.1.gz
