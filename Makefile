CC=clang
CFLAGS=-lclang
DFLAGS=-g

all: common dumptree

debug: debug_common debug_dumptree

dumptree: common
	$(CC) src/dumptree.c $(CFLAGS) bin/common.o -o bin/dumptree

common:
	$(CC) -c src/common.c $(CFLAGS) -o bin/common.o

debug_dumptree: debug_common
	$(CC) src/dumptree.c $(CFLAGS) $(DFLAGS) -o dumptree

debug_common: 
	$(CC) -c src/common.c $(CFLAGS) $(DFLAGS) -o bin/common.o

clean:
	rm -f bin/*


