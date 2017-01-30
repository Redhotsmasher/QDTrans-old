CC=$(COMPILER)
CFLAGS=-std=c11 $(LCLANG)
DFLAGS=-g -Wall -Wextra
MDFLAGS=-fsanitize=address
OPTLEV=0
DOPTLEV=0


build: main utils

all: main utils tests

utils: common.o dumptree printer.o printtree

tests: test0 testzero test1 test2 test3 test4 test5 test6 test7 test8 test9

debug: debug_common.o debug_dumptree debug_printer.o debug_printtree debug_qdtrans.o debug_main

mdebug: mdebug_common.o mdebug_dumptree mdebug_printer.o mdebug_printtree mdebug_qdtrans.o mdebug_main



dumptree: common.o
	$(CC) src/dumptree.c -O$(OPTLEV) bin/common.o $(CFLAGS) -o bin/dumptree

printtree: common.o printer.o
	$(CC) src/printtree.c -O$(OPTLEV) bin/common.o bin/printer.o $(CFLAGS) -o bin/printtree

qdtrans.o: common.o printer.o
	$(CC) -c src/qdtrans.c -O$(OPTLEV) $(CFLAGS) -o bin/qdtrans.o

printer.o: common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) -o bin/printer.o

common.o:
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) -o bin/common.o

main: common.o printer.o qdtrans.o
	$(CC) src/main.c -O$(OPTLEV) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/qdtrans

debug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(DOPTLEV) $(DFLAGS) bin/common.o $(CFLAGS) -o bin/dumptree

debug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o $(CFLAGS) -o bin/printtree

debug_qdtrans.o: debug_common.o debug_printer.o
	$(CC) -c src/qdtrans.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/qdtrans.o

debug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/printer.o

debug_common.o: 
	$(CC) -c src/common.c -O$(DOPTLEV) $(CFLAGS) $(DFLAGS) -o bin/common.o

debug_main: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) src/main.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/qdtrans

mdebug_dumptree: debug_common.o
	$(CC) src/dumptree.c -O$(OPTLEV) $(DFLAGS) $(MDFLAGS) bin/common.o $(CFLAGS) -o bin/dumptree

mdebug_printtree: debug_common.o debug_printer.o
	$(CC) src/printtree.c -O$(OPTLEV) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o $(CFLAGS) -o bin/printtree

mdebug_qdtrans.o: debug_common.o
	$(CC) -c src/qdtrans.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/qdtrans.o

mdebug_printer.o: debug_common.o
	$(CC) -c src/printer.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/printer.o

mdebug_common.o: 
	$(CC) -c src/common.c -O$(OPTLEV) $(CFLAGS) $(DFLAGS) $(MDFLAGS) -o bin/common.o

mdebug_main: mdebug_common.o mdebug_printer.o mdebug_qdtrans.o
	$(CC) src/main.c -O$(OPTLEV) $(DFLAGS) $(MDFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/qdtrans

cleandebug:
	rm -f bin/*

clean:
	rm -f tests/testfiles/*.qd.c
	rm -f bin/*


test0: test0a test0b test0c

test0a: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0a.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test0a

test0b: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0b.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test0b

test0c: debug_common.o debug_printer.o debug_qdtrans.o
	$(CC) tests/Test0c.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test0c


testzero: debug_main
	./bin/qdtrans ./tests/testfiles/Test0.c
	$(CC) tests/testfiles/Test0.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test0 -lpthread

test1: debug_main
	./bin/qdtrans ./tests/testfiles/Test1.c
	$(CC) tests/testfiles/Test1.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test1 -lpthread

test2: debug_main
	./bin/qdtrans ./tests/testfiles/Test2.c
	$(CC) tests/testfiles/Test2.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test2 -lpthread

test3: debug_main
	./bin/qdtrans ./tests/testfiles/Test3.c
	$(CC) tests/testfiles/Test3.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test3 -lpthread

test4: debug_main
	./bin/qdtrans ./tests/testfiles/Test4.c
	$(CC) tests/testfiles/Test4.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test4 -lpthread

test5: debug_main
	./bin/qdtrans ./tests/testfiles/Test5.c
	$(CC) tests/testfiles/Test5.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test5 -lpthread

test6: debug_main
	./bin/qdtrans ./tests/testfiles/Test6.c
	$(CC) tests/testfiles/Test6.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test6 -lpthread

test7: debug_main
	./bin/qdtrans ./tests/testfiles/Test7.c
	$(CC) tests/testfiles/Test7.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test7 -lpthread

test8: debug_main
	./bin/qdtrans ./tests/testfiles/Test8.c
	$(CC) tests/testfiles/Test8.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test8 -lpthread

test9: debug_main
	./bin/qdtrans ./tests/testfiles/Test9.c
	$(CC) tests/testfiles/Test9.qd.c -O$(DOPTLEV) $(DFLAGS) bin/common.o bin/printer.o bin/qdtrans.o $(CFLAGS) -o bin/test9 -lpthread
