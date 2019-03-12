/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26
#define MAGIC_NUMBER 0xfa19283e
#define FILENAME "./FILE_FS"
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

static int getSize(struct metadata md){
	int file_size = md.block_size;
	if (md.next == 0){
		return file_size;
	} else {
		while(md.next != 0){
			lseek(fd, md.next*BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			file_size += md.block_size;
		}
		return file_size;
	}
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr\n");
	int res = 0;
	int file_found = 0;

	struct metadata md;
	for (int i = 0; i < NUM_BLOCKS && strcmp(path, "/") != 0; ++i)
	{
		if(bitmap[i] == 1){
			lseek(fd, i*BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(path, md.filename) == 0){
				int file_found = 1;
				break;
			}
		}
	}

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if (file_found == 1)
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_birthtim = md.create_time; //creation time
		stbuf->st_atim = md.access_time;	//access time
		stbuf->st_mtim = md.modify_time;	//modification time	
		stbuf->st_size = getSize(md);
	}
	else
	{
		printf("Could not find attributes for the file %s\n", path);
		res = -ENOENT;
	}
		

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
	printf("open\n");
	struct metadata md;
	fd = open(FILENAME, O_RDONLY);
	for(int i = 1; i < NUM_BLOCKS; i++){
		if(bitmap[i] == 1){
			lseek(fd, BLOCK_SIZE * i, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(md.filename, path) == 0){
				clock_gettime(CLOCK_REALTIME, &md.access_time);
				write(fd, &md, sizeof(struct metadata));
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
	size_t read_size;
	char *block_buffer;
	(void)fi;
	off_t saved_offset = lseek(fd, 0, SEEK_CUR);

	struct metadata md;
	read(fd, &md, sizeof(struct metadata));

	/*
		Checks to see if the total size of the file minus the offset
		is smaller than the size we are trying to read and if so we
		return an error that an argument is invalid
	*/ 
	int file_size = getSize(md);
	if((int) size > (file_size - (int) offset)){
		return -EINVAL;
	}

	/*
		Now checks to see if the offset starts in a block of the file
		other than the first block and sets the file offset to the
		start of the file that the offset is in
	*/ 
	if((int) offset > USABLE_SPACE){
		if (md.next == 0){
			return -EINVAL;
		} else {
			int file_blocks = (file_size/USABLE_SPACE);
			int block_offset = ((int) offset/USABLE_SPACE);
			if (file_blocks < block_offset){
				return -EINVAL;
			} else {
				for (int i = 1; i < file_blocks; i++){
					lseek(fd, md.next*BLOCK_SIZE, SEEK_SET);
					if (i == block_offset){
						offset = offset - (off_t) (i*USABLE_SPACE);
						break;
					}
					read(fd, &md, sizeof(struct metadata));
				}
			}
		}
	} else {
		lseek(fd, saved_offset, SEEK_SET);
	}

	read(fd, &md, sizeof(struct metadata));
	lseek(fd, offset + sizeof(struct metadata), SEEK_CUR);

	/*
		If size is bigger than remaining space after offset
		then we read in only that portion from out block, else
		we just read that little snippet
	*/
	if(size > (USABLE_SPACE - offset)){
		read_size = USABLE_SPACE - offset;
		size = size - read_size;
		read(fd, block_buffer, read_size);
		strcat(buf, block_buffer);
	} else {
		read(fd, buf, size);
		size = 0;
	}

	/*
		If the size that we want to read spans across mutliple
		blocks of the file, then we go looking
	*/
	while(md.next != 0 && size > 0){
		lseek(fd, md.next*BLOCK_SIZE, SEEK_SET);
		read(fd, &md, sizeof(struct metadata));	
		if(size > USABLE_SPACE){
			size = size - USABLE_SPACE;
			read(fd, block_buffer, USABLE_SPACE);
			strcat(buf, block_buffer);
		} else {
			read(fd, buf, size);
			strcat(buf, block_buffer);
			break;
		}
	}
	lseek(fd, saved_offset, SEEK_SET);
	read(fd, &md, sizeof(struct metadata));
	clock_gettime(CLOCK_REALTIME, &md.access_time);
	write(fd, &md, sizeof(struct metadata)); 

	return sizeof(buf);
}

int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("write\n");
	return 0;
}

int hello_unlink(const char *path)
{
	printf("unlink\n");
	struct metadata md;
	for (int i = 0; i < NUM_BLOCKS; ++i)
	{
		if(bitmap[i] == 1){
			lseek(fd, i*BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(path, md.filename) == 0){
				bitmap[i] = 0;
				while(md.next != 0){
					bitmap[md.next] = 0;
					lseek(fd, md.next*BLOCK_SIZE, SEEK_SET);
					read(fd, &md, sizeof(struct metadata));					
				}
				lseek(fd, 4, SEEK_SET);
				write(fd, bitmap, sizeof(bitmap));
				return 0;
			}
		}
	}
	printf("Could not remove the file %s\n", path);
	return -ENOENT;
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
	fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
	ftruncate(fd, BLOCK_SIZE * NUM_BLOCKS);

	int32_t magic_number = 0xfa19283e;
	int32_t magic_test;
	read(fd, &magic_test, 4);
	if (magic_test == 0) {
		bitmap[0] = 1;
		lseek(fd, 0, SEEK_SET);
		write(fd, &magic_number, 4);
		write(fd, bitmap, sizeof(bitmap));
	}
	lseek(fd, 4, SEEK_SET);
	read(fd, bitmap, sizeof(bitmap));
	close(fd);
	return fuse_main(argc, argv, &hello_oper, NULL);
}
