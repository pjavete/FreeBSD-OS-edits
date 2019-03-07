all: 
	kldload fuse
	cc hello.c -o hello `pkgconf fuse --cflags --libs`
	mkdir newHelloFS
	./hello -f newHelloFS

clean:
	rm hello
	rm -rf newHelloFS
