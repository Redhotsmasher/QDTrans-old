CC=clang
CFLAGS=-lclang
DFLAGS=-g

dumptree:
	$(CC) src/dumptree.c $(CFLAGS) -o dumptree

debug:
	$(CC) src/dumptree.c $(CFLAGS) $(DFLAGS) -o dumptree

clean:
	rm -f dumptree


