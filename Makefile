CC=clang
CFLAGS=-std=c11 -lclang
DFLAGS=-g

all: common.o dumptree printer.o printtree qdtrans

debug: debug_common.o debug_dumptree debug_printer.o debug_printtree debug_qdtrans

dumptree: common.o
	$(CC) src/dumptree.c $(CFLAGS) bin/common.o -o bin/dumptree

printtree: common.o
	$(CC) src/printtree.c $(CFLAGS) bin/common.o bin/printer.o -o bin/printtree

qdtrans: common.o printer.o
	$(CC) src/qdtrans.c $(CFLAGS) bin/common.o bin/printer.o -o bin/qdtrans

printer.o: common.o
	$(CC) -c src/printer.c $(CFLAGS) -o bin/printer.o

common.o:
	$(CC) -c src/common.c $(CFLAGS) -o bin/common.o

debug_dumptree: debug_common.o
	$(CC) src/dumptree.c $(CFLAGS) $(DFLAGS) bin/common.o -o bin/dumptree

debug_printtree: debug_common.o
	$(CC) src/printtree.c $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o -o bin/printtree

debug_qdtrans: debug_common.o debug_printer.o
	$(CC) src/qdtrans.c $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o -o bin/qdtrans

debug_printer.o: debug_common.o
	$(CC) -c src/printer.c $(CFLAGS) $(DFLAGS) -o bin/printer.o

debug_common.o: 
	$(CC) -c src/common.c $(CFLAGS) $(DFLAGS) -o bin/common.o

clean:
	rm -f bin/*
