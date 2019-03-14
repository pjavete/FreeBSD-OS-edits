all: compile mount

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
	rm hello
	rm -rf newHelloFS