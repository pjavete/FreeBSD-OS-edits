all: compile mount

compile:
	cc hello.c -o hello `pkgconf fuse --cflags --libs`

mount:
	kldload fuse
	mkdir newHelloFS
	./hello -f newHelloFS

clean:
	rm hello
	rm -rf newHelloFS
