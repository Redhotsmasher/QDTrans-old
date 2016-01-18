CC=clang
CFLAGS=-lclang
DFLAGS=-g

all: common dumptree printtree

debug: debug_common debug_dumptree debug_printtree

dumptree: common
	$(CC) src/dumptree.c $(CFLAGS) bin/common.o -o bin/dumptree

printtree: common
	$(CC) src/printtree.c $(CFLAGS) bin/common.o -o bin/printtree

common:
	$(CC) -c src/common.c $(CFLAGS) -o bin/common.o

debug_dumptree: debug_common
	$(CC) src/dumptree.c $(CFLAGS) $(DFLAGS) bin/common.o -o bin/dumptree

debug_printtree: debug_common
	$(CC) src/printtree.c $(CFLAGS) $(DFLAGS) bin/common.o -o bin/printtree

debug_common: 
	$(CC) -c src/common.c $(CFLAGS) $(DFLAGS) -o bin/common.o

clean:
	rm -f bin/*
