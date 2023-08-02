COPT_O1=-O1 -march=native
COPT_O2=-O2 -march=native
COPT_O3=-O3 -march=native

LDOPTS=-lcrypto

MLEN=1024

CPUNUM=0

all: bench

tar:
	echo "NOT IMPL"

debug: bench.c zlr.c zlr.h deoxysBC-256.c deoxysBC-256.h util.c util.h
	$(CC) -g -O3 -march=native bench.c deoxysBC-256.c zlr.c util.c $(LDOPTS) -o test_debug
	echo "r 1024 1024\nbacktrace" | gdb test_debug

bench: bench.c zlr.c zlr.h deoxysBC-256.c deoxysBC-256.h util.c util.h
	$(CC) $(COPT_O1) bench.c deoxysBC-256.c zlr.c util.c $(LDOPTS) -o test_O1
	$(CC) $(COPT_O2) bench.c deoxysBC-256.c zlr.c util.c $(LDOPTS) -o test_O2
	$(CC) $(COPT_O3) bench.c deoxysBC-256.c zlr.c util.c $(LDOPTS) -o test_O3

bench1: test_O1
	taskset --cpu-list $(CPUNUM) ./test_O1 $(MLEN)

bench2: test_O2
	taskset --cpu-list $(CPUNUM) ./test_O2 $(MLEN)

bench3: test_O3
	taskset --cpu-list $(CPUNUM) ./test_O3 $(MLEN)

clean:
	rm -f test_debug
	rm -f test_O1
	rm -f test_O2
	rm -f test_O3
