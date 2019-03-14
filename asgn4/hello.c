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
#define BLOCK_SIZE 100
#define NUM_BLOCKS 10
#define MAX_BLOCKS ((BLOCK_SIZE - 4) / 4)
#define MAX_FILENAME_LENGTH 20
#define MAX_REQUEST_SIZE (128 * 1024)

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
	int file_size;
	struct timespec create_time;
	struct timespec access_time;
	struct timespec modify_time;
	int next;
};

#define MIN_BLOCK_SIZE (sizeof(struct metadata) + 1)
#define USABLE_SPACE (BLOCK_SIZE - sizeof(struct metadata))

static int hello_getattr(const char *path, struct stat *stbuf)
{
	printf("getattr for %s\n", path);
	int res = 0;
	int file_found = 0;

	fd = open(FILENAME, O_RDONLY);
	lseek(fd, 4, SEEK_SET);
	read(fd, &bitmap, sizeof(bitmap));

	struct metadata md;
	if (strcmp(path, "/") != 0)
	{
		for (int i = 1; i < NUM_BLOCKS; i++)
		{
			if(bitmap[i] == 1){
				lseek(fd, i*BLOCK_SIZE, SEEK_SET);
				read(fd, &md, sizeof(struct metadata));
				if(strcmp(path, md.filename) == 0){
					file_found = 1;
					break;
				}
			}
		}
	}

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if (strcmp(path, hello_path) == 0) //TEMPORARY
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	}
	else if (file_found == 1)
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_birthtim = md.create_time; //creation time
		stbuf->st_atim = md.access_time;	//access time
		stbuf->st_mtim = md.modify_time;	//modification time	
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = md.file_size;
	}
	else
	{
		printf("Could not find attributes for the file %s\n", path);
		res = -ENOENT;
	}
		
	close(fd);

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
						 off_t offset, struct fuse_file_info *fi)
{
	printf("readdir for %s\n", path);
	(void)offset;
	(void)fi;

	fd = open(FILENAME, O_RDONLY);

	struct metadata md;

	if (strcmp(path, "/") != 0){
		close(fd);
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	for (int i = 1; i < NUM_BLOCKS; i++)
	{
		if(bitmap[i] == 1){
			lseek(fd, i*BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(md.filename, "") != 0){
				filler(buf, md.filename + 1, NULL, 0);
			}
		}
	}

	close(fd);

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
				close(fd);
				return 0;
			}
		}
	}
	close(fd);
	return -ENOENT;

}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
					  struct fuse_file_info *fi)
{
	printf("read\n");
	(void)fi;

	fd = open(FILENAME, O_RDWR);

	int file_size;
	int file_start = 0;
	int byte_count = 0;
	char buffer[MAX_REQUEST_SIZE];

	struct metadata md;
	for (int i = 1; i < NUM_BLOCKS; i++)
	{
		if(bitmap[i] == 1){
			lseek(fd, i * BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(path, md.filename) == 0){
				file_size = md.file_size;
				file_start = i;
				break;
			}
		}
	}

	if (file_start == 0){
		close(fd);
		return -ENOENT;
	}

	lseek(fd, (file_start * BLOCK_SIZE) + sizeof(struct metadata), SEEK_SET);
	printf("read1\n");
	while (1)
	{
		if (read(fd, buffer + byte_count, USABLE_SPACE) == -1){
			close(fd);
			return -ENOBUFS;
		}
		byte_count += USABLE_SPACE;
		if (md.next != 0){
			lseek(fd, md.next * BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
		}else{
			break;
		}
	}
	printf("buffer = %s\n", buffer);
	printf("read2\n");
	if (offset < file_size)
	{
		if (offset + size > file_size)
			size = file_size - offset;
		memcpy(buf, buffer + offset, size);
	}
	else
		size = 0;
	printf("buf = %s\n", buf);
	printf("read3\n");
	clock_gettime(CLOCK_REALTIME, &md.access_time);
	lseek(fd, file_start * BLOCK_SIZE, SEEK_SET);
	write(fd, &md, sizeof(struct metadata));

	close(fd);

	printf("read success\n");

	return size;
}

int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("write\n");
	(void)fi;

	int new_file_size;
	int file_start = 0;
	int byte_count = 0;
	int bytes_written = 0;
	char buffer[MAX_REQUEST_SIZE];

	fd = open(FILENAME, O_RDWR);

	struct metadata md;
	for (int i = 1; i < NUM_BLOCKS; i++)
	{
		if(bitmap[i] == 1){
			lseek(fd, i * BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(path, md.filename) == 0){
				file_start = i;
				break;
			}
		}
	}

	if (file_start == 0){
		close(fd);
		return -ENOENT;
	}

	lseek(fd, (file_start * BLOCK_SIZE) + sizeof(struct metadata), SEEK_SET);
	while (1)
	{
		if (read(fd, buffer + byte_count, USABLE_SPACE) == -1){
			close(fd);
			return -ENOBUFS;
		}
		byte_count += USABLE_SPACE;
		if (md.next != 0){
			lseek(fd, md.next * BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
		}else{
			break;
		}
	}

	new_file_size = offset + size;
	if (new_file_size > MAX_REQUEST_SIZE){
		close(fd);
		return -ENOBUFS;
	}

	memcpy(buffer + offset, buf, size);
	lseek(fd, file_start * BLOCK_SIZE, SEEK_SET);
	read(fd, &md, sizeof(struct metadata));
	md.file_size = new_file_size;
	clock_gettime(CLOCK_REALTIME, &md.access_time);
	clock_gettime(CLOCK_REALTIME, &md.modify_time);
	lseek(fd, file_start * BLOCK_SIZE, SEEK_SET);
	write(fd, &md, sizeof(struct metadata));

	int current_block = file_start;
	int next_block = file_start;

	lseek(fd, (file_start * BLOCK_SIZE) + sizeof(struct metadata), SEEK_SET);
	while(1)
	{
		if (new_file_size > USABLE_SPACE){
			if (md.next == 0){
				int nextAvailableBlock = 0;
				for (int i = 1; i < NUM_BLOCKS; i++) {
					if (bitmap[i] == 0){
						nextAvailableBlock = i;
						bitmap[i] = 1;
						md.next = i;
						lseek(fd, current_block * BLOCK_SIZE, SEEK_SET);
						write(fd, &md, sizeof(struct metadata));
						next_block = md.next;
						memset(&md, 0, sizeof(struct metadata));
						break;
					}
				}
				if (nextAvailableBlock == 0){
					close(fd);
					return -ENOMEM;
				}
			}else{
				lseek(fd, current_block * BLOCK_SIZE, SEEK_SET);
				read(fd, &md, sizeof(struct metadata));
				next_block = md.next;
				lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
				read(fd, &md, sizeof(struct metadata));
			}
			lseek(fd, (current_block * BLOCK_SIZE) + sizeof(struct metadata), SEEK_SET);
			write(fd, buffer + bytes_written, USABLE_SPACE);
			bytes_written += USABLE_SPACE;
			new_file_size -= USABLE_SPACE;
			current_block = next_block;
		}else{
			lseek(fd, (current_block * BLOCK_SIZE) + sizeof(struct metadata), SEEK_SET);
			write(fd, buffer + bytes_written, new_file_size);
			bytes_written += new_file_size;
			new_file_size = 0;
			break;
		}
	}
	lseek(fd, 4, SEEK_SET);
	write(fd, &bitmap, sizeof(bitmap));

	close(fd);

	printf("bytes written: %d\n", bytes_written);

	return bytes_written;
}

int hello_unlink(const char *path)
{
	printf("unlink\n");

	fd = open(FILENAME, O_RDWR);

	lseek(fd, 4, SEEK_SET);
	read(fd, &bitmap, sizeof(bitmap));

	struct metadata md;
	for (int i = 1; i < NUM_BLOCKS; i++)
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
				write(fd, &bitmap, sizeof(bitmap));
				close(fd);
				return 0;
			}
		}
	}
	close(fd);
	printf("Could not remove the file %s\n", path);
	return -ENOENT;
}

int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("create\n");
	(void)mode;
	(void)fi;
	struct metadata md;

	fd = open(FILENAME, O_RDWR);
	lseek(fd, 4, SEEK_SET);
	read(fd, &bitmap, sizeof(bitmap));

	for (int i = 1; i < NUM_BLOCKS; i++) {
		if (bitmap[i] == 1){
			lseek(fd, i * BLOCK_SIZE, SEEK_SET);
			read(fd, &md, sizeof(struct metadata));
			if(strcmp(path, md.filename) == 0){
				close(fd);
				return -EEXIST;
			}
		}
	}

	int nextAvailableBlock = 0;
	for (int i = 1; i < NUM_BLOCKS; i++) {
		if (bitmap[i] == 0){
			nextAvailableBlock = i;
			bitmap[i] = 1;
			lseek(fd, 4, SEEK_SET);
			write(fd, &bitmap, sizeof(bitmap));
			break;
		}
	}
	if (nextAvailableBlock == 0){
		close(fd);
		return -ENOMEM;
	}
	long offset = nextAvailableBlock * BLOCK_SIZE;
	if (strnlen(path, MAX_FILENAME_LENGTH) == MAX_FILENAME_LENGTH){
		close(fd);
		return -ENAMETOOLONG;
	}
	strcpy(md.filename, path);
	md.file_size = 0;
	clock_gettime(CLOCK_REALTIME, &md.create_time);
	clock_gettime(CLOCK_REALTIME, &md.modify_time);
	md.next = 0;
	lseek(fd, offset, SEEK_SET);
	write(fd, &md, sizeof(struct metadata));
	close(fd);
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
	if (BLOCK_SIZE < MIN_BLOCK_SIZE)
	{
		perror("BLOCK_SIZE is too small");
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
