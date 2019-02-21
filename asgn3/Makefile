all: benchmark

benchmark: benchmark.o
	cc -Wall -std=c99 -o benchmark benchmark.c

benchmark.o: benchmark.c
	cc -Wall -std=c99 -c benchmark.c

clean:
	rm benchmark benchmark.o

backup:
	cp /sys/vm/vm_page.c /sys/vm/vm_page.c.backup
	cp /sys/vm/vm_pageout.c /sys/vm/vm_pageout.c.backup
	cp /sys/sys/vm_page.h /sys/sys/vm_page.h.backup

restore:
	mv /sys/vm/vm_page.c.backup /sys/vm/vm_page.c
	mv /sys/vm/vm_pageout.c.backup /sys/vm/vm_pageout.c
	mv /sys/vm/vm_page.h.backup /sys/vm/vm_page.h

move:
	cp vm_page.c /sys/vm/
	cp vm_pageout.c /sys/vm/
	cp vm_page.h /sys/vm/
