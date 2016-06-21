CC=clang
CFLAGS=-std=c11 -lclang -I /usr/lib/llvm-3.6/include/
DFLAGS=-g -Wall -Wextra
MDFLAGS=-fsanitize=address
OPTLEV=3
DOPTLEV=0

all: common.o dumptree printer.o printtree qdtrans

debug: debug_common.o debug_dumptree debug_printer.o debug_printtree debug_qdtrans

mdebug: mdebug_common.o mdebug_dumptree mdebug_printer.o mdebug_printtree mdebug_qdtrans

dumptree: common.o
	$(CC) src/dumptree.c -O$(OPTLEV) $(CFLAGS) bin/common.o -o bin/dumptree

printtree: common.o printer.o
	$(CC) src/printtree.c -O$(OPTLEV) $(CFLAGS) bin/common.o bin/printer.o -o bin/printtree

qdtrans: common.o printer.o
	$(CC) src/qdtrans.c -O$(OPTLEV) $(CFLAGS) bin/common.o bin/printer.o -o bin/qdtrans

printer.o: common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) -o bin/printer.o

common.o:
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) -o bin/common.o

debug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o -o bin/dumptree

debug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o -o bin/printtree

debug_qdtrans: debug_common.o debug_printer.o
	$(CC) src/qdtrans.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o -o bin/qdtrans

debug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/printer.o

debug_common.o: 
	$(CC) -c src/common.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/common.o

mdebug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o -o bin/dumptree

mdebug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o -o bin/printtree

mdebug_qdtrans: debug_common.o debug_printer.o
	$(CC) src/qdtrans.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o -o bin/qdtrans

mdebug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/printer.o

mdebug_common.o: 
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/common.o

clean:
	rm -f bin/*
