all: benchmark

benchmark: benchmark.o
	gcc -Wall -std=c99 -o benchmark benchmark.c

benchmark.o: benchmark.c
	gcc -Wall -std=c99 -c benchmark.c

clean:
	rm benchmark benchmark.o

backup:
	cp /sys/kern/sched_ule.c /sys/kern/sched_ule.c.backup
	cp /sys/kern/kern_switch.c /sys/kern/kern_switch.c.backup
	cp /sys/sys/proc.h /sys/sys/proc.h.backup
	cp /sys/sys/runq.h /sys/sys/runq.h.backup

restore:
	mv /sys/kern/sched_ule.c.backup /sys/kern/sched_ule.c
	mv /sys/kern/kern_switch.c.backup /sys/kern/kern_switch.c
	mv /sys/sys/proc.h.backup /sys/sys/proc.h
	mv /sys/sys/runq.h.backup /sys/sys/runq.h

move:
	cp sched_ule.c /sys/kern/
	cp kern_switch.c /sys/kern/
	cp proc.h /sys/sys/
	cp runq.h /sys/sys/
