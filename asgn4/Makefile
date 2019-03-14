all: compile mount benchmark

compile:
	cc hello.c -o hello `pkgconf fuse --cflags --libs`

benchmark: benchmark.o
	cc -Wall -std=c99 -o benchmark benchmark.c

benchmark.o: benchmark.c
	cc -Wall -std=c99 -c benchmark.c

mount:
	kldload fuse
	mkdir newHelloFS
	./hello -f newHelloFS

clean:
	rm hello FILE_FS benchmark benchmark.o
	rm -rf newHelloFS