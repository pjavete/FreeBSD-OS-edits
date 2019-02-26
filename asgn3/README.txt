# chtzou
# pjavete
# dflung

#Files included:
vm_page.c
vm_page.h
vm_pageout.c
syslog.conf
asgn3_stats.log
Makefile
README.txt
design_document.txt

How to complie and run:
	1. transfer all files to a folder on your FreeBSD system
	2. type - sudo make move - on the command line, this moves all of the files to the correct places
	3. recompile and install the kernel, and reboot
	4. install the stress package on FreeBSD by typing - sudo pkg install stress - on the command line
	5. run the stress test by typing - stress -c 2 -i 1 -m 1 --vm-bytes 3594M -t 10s - on the command line
	6. You may have to adjust the number before M based on the amount of memory on your VM. It represents doing a Malloc of x MB per vm worker. Additionally it may crash the VM but when it boots back up you should be able to run it just fine. This stress test can be a little finicky.
	7. Look at the file /var/log/asgn3_stats for printed statistics