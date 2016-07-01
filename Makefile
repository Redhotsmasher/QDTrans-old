CC=$(COMPILER)
CFLAGS=-std=c11 -lclang -I /usr/lib/llvm-3.6/include/
DFLAGS=-g -Wall -Wextra
MDFLAGS=-fsanitize=address
OPTLEV=3
DOPTLEV=0


build: main utils

all: main utils tests

utils: common.o dumptree printer.o printtree

tests: test0

debug: debug_common.o debug_dumptree debug_printer.o debug_printtree debug_qdtrans.o debug_main

mdebug: mdebug_common.o mdebug_dumptree mdebug_printer.o mdebug_printtree mdebug_qdtrans.o mdebug_main



dumptree: common.o
	$(CC) src/dumptree.c -O$(OPTLEV) $(CFLAGS) bin/common.o -o bin/dumptree

printtree: common.o printer.o
	$(CC) src/printtree.c -O$(OPTLEV) $(CFLAGS) bin/common.o bin/printer.o -o bin/printtree

qdtrans.o: common.o printer.o
	$(CC) -c src/qdtrans.c -O$(OPTLEV) $(CFLAGS) -o bin/qdtrans.o

printer.o: common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) -o bin/printer.o

common.o:
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) -o bin/common.o

main: common.o printer.o qdtrans.o
	$(CC) src/main.c -O$(OPTLEV) $(CFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/qdtrans

debug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o -o bin/dumptree

debug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o -o bin/printtree

debug_qdtrans.o: debug_common.o debug_printer.o
	$(CC) -c src/qdtrans.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/qdtrans.o

debug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/printer.o

debug_common.o: 
	$(CC) -c src/common.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/common.o

debug_main: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) src/main.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/qdtrans

mdebug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o -o bin/dumptree

mdebug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o -o bin/printtree

mdebug_qdtrans.o: debug_common.o
	$(CC) -c src/qdtrans.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/qdtrans.o

mdebug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/printer.o

mdebug_common.o: 
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/common.o

mdebug_main: mdebug_common.o mdebug_printer.o mdebug_qdtrans.o
	$(CC) src/main.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/qdtrans

clean:
	rm -f bin/*



test0: test0a test0b test0c

test0a: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0a.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/test0a

test0b: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0b.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/test0b

test0c: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0c.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o -o bin/test0c
