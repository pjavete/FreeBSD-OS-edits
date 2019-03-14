#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define BILLION 1000000000L

int main(){
	const char file[] = "file";
	char filename[8];
	char buf[200];
	struct timespec time_now;
	FILE* fp;
	int fd;
	long start_nsec, end_nsec, runtime_nsec;
	time_t start_sec, end_sec, runtime_sec;

	printf("Testing FreeBSD file system\n");
	printf("---------------------------\n");
	printf("Creating 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 100; i++){
		sprintf(filename, "%s%d", file, i);
		fp = fopen(filename, "w+");
		fclose(fp);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffBSDCreate = BILLION * (runtime_sec) + runtime_nsec;
	printf("Created 100 files in %llu nanoseconds\n", (long long unsigned int) diffBSDCreate);

	printf("---------------------------\n");
	printf("Writing to 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 100; i++){
		sprintf(filename, "%s%d", file, i);
		fp = fopen(filename, "a");
		for (int x = 0; x < (i*2)+1; x++){
			fprintf(fp, "This is our test string. Hello grader! :)\n");
		}
		fclose(fp);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffBSDWrite = BILLION * (runtime_sec) + runtime_nsec;
	printf("Wrote to 100 files in %llu nanoseconds\n", (long long unsigned int) diffBSDWrite);

	printf("---------------------------\n");
	printf("Reading from 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 100; i++){
		sprintf(filename, "%s%d", file, i);
		fd = open(filename, O_RDONLY);
		lseek(fd, i*10, SEEK_SET);
		read(fd, buf, i*2);
		//printf("%s\n", buf);
		close(fd);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffBSDRead = BILLION * (runtime_sec) + runtime_nsec;
	printf("Read from 100 files in %llu nanoseconds\n", (long long unsigned int) diffBSDRead);

	printf("---------------------------\n");
	printf("Testing our implemented fuse file system\n");
	printf("---------------------------\n");

	chdir("newHelloFS");

	fflush(stdout);
	printf("Creating 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 100; i++){
		sprintf(filename, "%s%d", file, i);
		fp = fopen(filename, "w+");
		fclose(fp);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffFUSECreate = BILLION * (runtime_sec) + runtime_nsec;
	printf("Created 100 files in %llu nanoseconds\n", (long long unsigned int) diffFUSECreate);

	printf("---------------------------\n");
	printf("Writing to 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 98; i++){
		sprintf(filename, "%s%d", file, i);
		fp = fopen(filename, "a");
		for (int x = 0; x < (i*2)+1; x++){
			fprintf(fp, "This is our test string. Hello grader! :)\n");
		}
		fclose(fp);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffFUSEWrite = BILLION * (runtime_sec) + runtime_nsec;
	printf("Wrote to 100 files in %llu nanoseconds\n", (long long unsigned int) diffFUSEWrite);

	printf("---------------------------\n");
	printf("Reading from 100 files\n");
	clock_gettime(CLOCK_REALTIME, &time_now);
	start_sec = time_now.tv_sec;
	start_nsec = time_now.tv_nsec;
	for(int i = 0; i < 100; i++){
		sprintf(filename, "%s%d", file, i);
		fd = open(filename, O_RDONLY);
		lseek(fd, i*10, SEEK_SET);
		read(fd, buf, i*2);
		//printf("%s\n", buf);
		close(fd);
	}
	clock_gettime(CLOCK_REALTIME, &time_now);
	end_sec = time_now.tv_sec;
	end_nsec = time_now.tv_nsec;
	runtime_sec = end_sec-start_sec;
	runtime_nsec = end_nsec-start_nsec;
	uint64_t diffFUSERead = BILLION * (runtime_sec) + runtime_nsec;
	printf("Read from 100 files in %llu nanoseconds\n", (long long unsigned int) diffFUSERead);

	printf("---------------------------\n");
	printf("Some Statistics:\n");
	long diffCreate = ((diffFUSECreate - diffBSDCreate) / diffBSDCreate)*100;
	long diffWrite = ((diffFUSEWrite - diffBSDWrite) / diffBSDWrite)*100;
	long diffRead = ((diffFUSERead - diffBSDRead) / diffBSDRead)*100;
	long diff = (diffCreate + diffWrite + diffRead)/3;
	printf("Our Fuse file system is about %ld%% slower than the FreeBSD file system\n", diff);
	printf("---------------------------\n");
	printf("End of benchmark\n");
	return 0;
}