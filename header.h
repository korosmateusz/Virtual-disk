#ifndef HEADER_H_
#define HEADER_H_
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME_LENGTH 15
#define BLOCK_SIZE 2048
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE  "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct VirtualDisk {
	FILE* file;
	unsigned int inodesAmount;
	struct iNode *inodes;
};

struct SuperBlock {
	size_t diskSize;
};

struct iNode {
	char name[NAME_LENGTH];
	unsigned int size;
	unsigned int next;
	unsigned int isFirstOfFile;
	unsigned int isUsed;
};

const char* DISK_NAME;
struct VirtualDisk* createDisk(const size_t size);
void deleteDisk();
struct VirtualDisk* openDisk();
void closeDisk (struct VirtualDisk* disk);
int copyToDisk(struct VirtualDisk* disk,const char* driveFile, const char* diskFile);
int copyFromDisk(struct VirtualDisk* disk,const char* diskFile,const char* driveFile);
int removeFile(struct VirtualDisk* disk, const char* name);
void listFiles(struct VirtualDisk* disk);
void listBlocks (struct VirtualDisk* disk);

#endif
