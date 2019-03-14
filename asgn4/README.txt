# dflung
# chtzou
# pjavete

#Files included:
hello.c
benchmark.c
Makefile
README.txt
design_document.txt

How to complie and run:
	1. transfer all files to a folder on your FreeBSD system
	2. open up a seperate terminal and ssh into the VM
	3. in both the terminal and VM go to the folder and become a super user
	4. run make in the terminal, this terminal will become the place for printing
		out stuff from our FuseFS
	5. now run ./benchmark in the VM (note: nothing is printed when a file is read
		during the benchmark, you can uncomment line 71, and 137 if you wish for
		your terminal to be filled with text)
	6. before running benchmark again do rm file* in both your folder and the Fuse folder