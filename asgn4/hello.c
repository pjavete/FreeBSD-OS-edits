/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26
#define MAGIC_NUMBER 0xfa19283e
#define BLOCK_SIZE 4096
#define NUM_BLOCKS 100
#define MAX_BLOCKS ((BLOCK_SIZE - 4) / 4)
#define MAX_FILENAME_LENGTH 20

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int bitmap[NUM_BLOCKS];
static int fd;

struct metadata {
	char filename[MAX_FILENAME_LENGTH];
	int block_size;
	struct timespec create_time;
	struct timespec access_time;
	struct timespec modify_time;
	int next;
};

#define USABLE_SPACE (BLOCK_SIZE - sizeof(struct metadata))

static int hello_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr\n");
	int res = 0;
	//stbuf->st_birthtim = time(NULL); //creation time
	//stbuf->st_atime = time(NULL);	//access time
	//stbuf->st_mtime = time(NULL);	//modification time
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if (strcmp(path, hello_path) == 0)
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	}
	else
		res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
						 off_t offset, struct fuse_file_info *fi)
{
	printf("readdir\n");
	(void)offset;
	(void)fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	//filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	int offset;
	int open_status;
	struct metadata md;
	printf("open\n");
	//might need to manipulate the string to make sure it's the path?
	//char fpath[SOME_VALUE];
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;
	else{

	}
	/*
	//use read (fd, buf, size) and then check
	//iterate through bitmap and see if there a file that matches. (strcomp)
	//if yes, then open the file (lseek???)
	//log access time (use sizeof)
	//if not, call create
	for(int i = 0; i < NUM_BLOCKS; i++){
		if(bitmap[i] == 1){
			lseek(fd, offset * i, SEEK_SET);
			if(md.filename == path){
				clock_gettime(CLOCK_REALTIME, &md.access_time);
			}
		}
	}
	*/
	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
					  struct fuse_file_info *fi)
{
	//rs = read status
	int rs;
	printf("read\n");
	size_t len;
	(void)fi;
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len)
	{
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	}
	else
		size = 0;
	lseek(fd, offset + sizeof(metadata), SEEK_CUR);
	for(int i = 0; i != NULL; i++){
		printf(i);
	}
	if(next != NULL){
		//hello_read recursive call maybe?
	}
	return size;
}

int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("write\n");
	return 0;
}

int hello_unlink(const char *path)
{
	printf("unlink\n");
	return 0;
}

int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("create\n");
	(void)mode;
	(void)fi;
	int nextAvailableBlock = 0;
	for (int i = 0; i < NUM_BLOCKS; i++) {
		if (bitmap[i] == 0){
			nextAvailableBlock = i;
			bitmap[i] = 1;
			break;
		}
	}
	if (nextAvailableBlock == 0)
		return -ENOMEM;
	long offset = nextAvailableBlock * BLOCK_SIZE;
	if (strnlen(path, MAX_FILENAME_LENGTH) == MAX_FILENAME_LENGTH)
		return -ENAMETOOLONG;
	struct metadata md;
	strcpy(md.filename, path);
	md.block_size = 0;
	clock_gettime(CLOCK_REALTIME, &md.create_time);
	clock_gettime(CLOCK_REALTIME, &md.modify_time);
	lseek(fd, offset, SEEK_SET);
	write(fd, &md, sizeof(struct metadata));
	printf("create success\n");
	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
	.readdir = hello_readdir,
	.open = hello_open,
	.read = hello_read,
	.unlink = hello_unlink,
	.write = hello_write,
	.create = hello_create,
};

int main(int argc, char *argv[])
{
	if (NUM_BLOCKS > MAX_BLOCKS)
	{
		perror("NUM_BLOCKS exceeds maximum allowed amount of blocks");
		exit(1);
	}
	char *fs = "./FILE_FS";
	fd = open(fs, O_RDWR | O_APPEND | O_CREAT, 0666);
	ftruncate(fd, BLOCK_SIZE * NUM_BLOCKS);

	int32_t magic_number = MAGIC_NUMBER;
	int32_t magic_test;
	read(fd, &magic_test, 4);
	if (magic_test == 0) {
		bitmap[0] = 1;
		write(fd, magic_number, 4);
		write(fd, bitmap, sizeof(bitmap));
	}
	lseek(fd, 4, SEEK_SET);
	read(fd, bitmap, sizeof(bitmap));
	return fuse_main(argc, argv, &hello_oper, NULL);
}
