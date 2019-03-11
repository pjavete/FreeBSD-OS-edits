/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26
#define BLOCK_SIZE 4096
#define NUM_BLOCKS 100
#define MAX_BLOCKS 1023

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int bitmap[NUM_BLOCKS];

static int hello_getattr(const char *path, struct stat *stbuf)
{
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
	printf("open\n");
	int open_status;
	struct metadata md;
	//might need to manipulate the string to make sure it's the path?
	//char fpath[SOME_VALUE];
	for(int i = 0; i < NUM_BLOCKS; i++){
		if(bitmap[i] == 1){
			lseek(fd, BLOCK_SIZE * i, SEEK_SET);
			if(strcomp(md.file_name, path) == 0){
				clock_gettime(CLOCK_REALTIME, &md.access_time);
				return 0;
			}
		}
	}
	return -ENOENT;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
					  struct fuse_file_info *fi)
{
	printf("read\n");
	struct metadata md;
	size_t len;
	(void)fi;
	len = strlen(hello_str);
	//do we keep or delete?
	//i feel like we should delete
	if (offset < len)
	{
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	}
	else
		size = 0;
	while(1){
		lseek(fd, offset + sizeof(struct metadata), SEEK_SET);
		memcpy(buf, offset + sizeof(struct metadata), size);
		if(md.next == NULL){
			break;
		}
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
	bitmap[0] = 1;
	int fd;
	char *fs = "./FILE_FS";
	fd = open(fs, O_RDWR | O_APPEND | O_CREAT, 0666);
	ftruncate(fd, BLOCK_SIZE * NUM_BLOCKS);
	return fuse_main(argc, argv, &hello_oper, NULL);
}
