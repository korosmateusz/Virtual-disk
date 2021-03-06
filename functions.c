#include "header.h"

struct VirtualDisk* createDisk(const size_t size) {
	if (size < 2084) {
		printf(ANSI_COLOR_RED "Size too small to create disk.\n"ANSI_COLOR_RESET);
		return NULL;
	}
	FILE* file;
	char writingBlock[64];
	size_t writingBytes;
	size_t remainingBytes;
	struct VirtualDisk* disk;
	struct SuperBlock super;
	unsigned int inodesAmount;
	struct iNode* inodes;
	unsigned int i;
	file = fopen(DISK_NAME,"wb");
	if(!file)
		return NULL;
	memset(writingBlock, 0, sizeof(writingBlock));
	remainingBytes = size;
	while(remainingBytes > 0) {
		writingBytes = sizeof(writingBlock);
		if(writingBytes > remainingBytes)
			writingBytes = remainingBytes;
		fwrite(writingBlock, 1, writingBytes, file);
		remainingBytes -= writingBytes;
	}
	fseek(file, 0, SEEK_SET);
	super.diskSize = size;
	fwrite(&super, sizeof(struct SuperBlock), 1, file);
	inodesAmount = (size-sizeof(struct SuperBlock))/(sizeof(struct iNode)+BLOCK_SIZE);
	inodes = malloc(sizeof(struct iNode)*inodesAmount);
	for(i = 0; i < inodesAmount; i++) {
		inodes[i].isFirstOfFile = 0;
		inodes[i].isUsed = 0;
	}
	disk = malloc(sizeof(struct VirtualDisk));
	disk->file = file;
	disk->inodesAmount = inodesAmount;
	disk->inodes = inodes;
	return disk;
}

void deleteDisk() {
	unlink(DISK_NAME);
}

struct VirtualDisk* openDisk() {
	FILE* file;
	size_t size;
	struct SuperBlock super;
	unsigned int inodesAmount;
	struct iNode* inodes;
	struct VirtualDisk* disk;
	file = fopen(DISK_NAME,"r+b");
	if(!file)
		return NULL;
	fseek(file,0,SEEK_END);
	size = ftell(file);
	if(size < sizeof(struct SuperBlock)) {
		fclose(file);
		return NULL;
	}
	fseek(file,0,SEEK_SET);
	if(fread(&super, sizeof(struct SuperBlock), 1, file) <= 0) {
		fclose(file);
		return NULL;
	}
	if(super.diskSize != size) {
		fclose(file);
		return NULL;
	}
	inodesAmount = (size-sizeof(struct SuperBlock))/(sizeof(struct iNode)+BLOCK_SIZE);
	inodes = malloc(inodesAmount*sizeof(struct iNode));
	if(fread(inodes, sizeof(struct iNode), inodesAmount, file) <= 0) {
		fclose(file);
		free(inodes);
		return NULL;
	}
	disk = malloc(sizeof(struct VirtualDisk));
	disk->file = file;
	disk->inodesAmount = inodesAmount;
	disk->inodes = inodes;
	return disk;
}

void closeDisk(struct VirtualDisk* disk) {
	fseek(disk->file, sizeof(struct SuperBlock), SEEK_SET);
	fwrite(disk->inodes, sizeof(struct iNode), disk->inodesAmount, disk->file);
	fclose(disk->file);
	free(disk->inodes);
	free(disk);
	disk = NULL;	
}

int copyToDisk(struct VirtualDisk* disk, const char* driveFile, const char* diskFile) {
	FILE* file;
	unsigned int i;
	unsigned int iNodesToCreate = 1;
	unsigned int inodesIterator;
	size_t sourceSize;
	unsigned int* InodesToFill;
	char buffer[BLOCK_SIZE];
	if (strlen(driveFile) == 0 || strlen(diskFile) == 0) {
		printf(ANSI_COLOR_RED "Please specify a file.\n"ANSI_COLOR_RESET);
		return 1;
	}
	if (strlen(driveFile) > NAME_LENGTH || strlen(diskFile) > NAME_LENGTH) {
		printf(ANSI_COLOR_RED "File name too long, please choose shorter name.\n"ANSI_COLOR_RESET);
		return 1;
	}
	for(i=0; i < disk->inodesAmount; i++) {
		if(disk->inodes[i].isUsed == 1 && strncmp(disk->inodes[i].name,diskFile,NAME_LENGTH) == 0) {
			printf(ANSI_COLOR_RED "File with such name already exists on the virtual disk.\n"ANSI_COLOR_RESET);
			return 1;
		}
	}
	file=fopen(driveFile,"rb");
	if(!file) {
		printf(ANSI_COLOR_RED "Couldn't open file.\n"ANSI_COLOR_RESET);
		return 1;
	}
	fseek(file,0,SEEK_END);
	sourceSize=ftell(file);
	fseek(file,0,SEEK_SET);
	if(!sourceSize) {
		iNodesToCreate = 1;
	}
	else if(iNodesToCreate % BLOCK_SIZE) {
		iNodesToCreate = sourceSize/BLOCK_SIZE + 1;
	}
	else {
		iNodesToCreate = sourceSize/BLOCK_SIZE;
	}
	InodesToFill = malloc(iNodesToCreate*sizeof(unsigned int));
	inodesIterator = 0;
	for(i = 0; i < disk->inodesAmount; i++) {
		if(disk->inodes[i].isUsed == 0)
			InodesToFill[inodesIterator++]=i;
		if(inodesIterator == iNodesToCreate)
			break;
	}
	if(inodesIterator < iNodesToCreate) {
		printf(ANSI_COLOR_RED "Not enough place on virtual disk.\n"ANSI_COLOR_RESET);
		return 1; 
	}
	for (i = 0; i < iNodesToCreate; i++) {
		disk->inodes[InodesToFill[i]].isUsed = 1;
		disk->inodes[InodesToFill[i]].size = fread(buffer, 1, sizeof(buffer), file);
		fseek(disk->file, sizeof(struct SuperBlock)+sizeof(struct iNode)*(disk->inodesAmount)+BLOCK_SIZE*InodesToFill[i], SEEK_SET);
		fwrite(buffer, 1, disk->inodes[InodesToFill[i]].size, disk->file);
		if(i == 0)
			disk->inodes[InodesToFill[i]].isFirstOfFile = 1;
		strncpy(disk->inodes[InodesToFill[i]].name,diskFile,NAME_LENGTH);
		if(i < iNodesToCreate-1)
			disk->inodes[InodesToFill[i]].next = InodesToFill[i+1];
		else 
			disk->inodes[InodesToFill[i]].next = -1;
	}
	free(InodesToFill);
	fclose(file);
	return 0;
}

int copyFromDisk(struct VirtualDisk* disk, const char* diskFile, const char* driveFile) {
	FILE* file;
	int startingNode = -1;
	int i;
	char buffer[BLOCK_SIZE];
	if (strlen(driveFile) > NAME_LENGTH || strlen(diskFile) > NAME_LENGTH) {
		printf(ANSI_COLOR_RED "File name too long, please choose shorter name.\n"ANSI_COLOR_RESET);
		return 1;
	}
	for (i = 0; i < disk->inodesAmount; i++) {
		if(disk->inodes[i].isFirstOfFile == 1 && disk->inodes[i].isUsed == 1 && strncmp(diskFile, disk->inodes[i].name, NAME_LENGTH) == 0) {
			startingNode=i;
			break;
		}
	}
	if(startingNode == -1) {
		printf(ANSI_COLOR_RED "Couldn't find file with such name.\n"ANSI_COLOR_RESET);
		return 1;
	}
	file=fopen(driveFile,"wb");
	if(!file) {
		printf(ANSI_COLOR_RED "Failed to create file.\n"ANSI_COLOR_RESET);
		return 1;
	}
	
	while(startingNode != -1) {
		fseek(disk->file, sizeof(struct SuperBlock)+sizeof(struct iNode)*(disk->inodesAmount)+BLOCK_SIZE*startingNode,SEEK_SET);
		if(fread(buffer, 1, disk->inodes[startingNode].size, disk->file) != disk->inodes[startingNode].size) {
			fclose(file);
			return 1;
		}
		fwrite(buffer, 1, disk->inodes[startingNode].size, file);
		startingNode = disk->inodes[startingNode].next;
	}
	fclose(file);
	return 0;
}

int removeFile(struct VirtualDisk* disk, const char* name) {
	unsigned int i;
	int startingNode = -1;
	for (i = 0; i < disk->inodesAmount; i++) {
		if(disk->inodes[i].isFirstOfFile == 1 && disk->inodes[i].isUsed == 1 && strncmp(name,disk->inodes[i].name,NAME_LENGTH) == 0) {
			startingNode = i;
			disk->inodes[i].isFirstOfFile = 0;
			break;
		}
	}
	if(startingNode == -1) {
		printf(ANSI_COLOR_RED "File with such name doesn't exist on virtual disk.\n"ANSI_COLOR_RESET);
		return 1;
	}
	while(startingNode != -1) {
		disk->inodes[startingNode].isUsed = 0;
		startingNode = disk->inodes[startingNode].next;
	}
	return 0;
}

void listFiles (struct VirtualDisk* disk) {
	unsigned int i,j;
	unsigned int size = 0;;
	int fileCount = 0;
	for(i = 0; i < disk->inodesAmount; i++) {
		if(disk->inodes[i].isFirstOfFile == 1 && disk->inodes[i].isUsed == 1) {
			fileCount++;
			printf(ANSI_COLOR_WHITE"File no. %d\n", fileCount);
			printf(" \\_Name: %s\t"ANSI_COLOR_CYAN, disk->inodes[i].name);
			printf("Inodes: %d",i);
			size=disk->inodes[i].size;		
			for(j=i+1;j<disk->inodesAmount;j++) {
				if(strncmp(disk->inodes[i].name,disk->inodes[j].name,NAME_LENGTH)==0) {
					printf(",%d",j);
					size+=disk->inodes[j].size;
				}
			}
			printf("\t\t");	
			printf("Size: %d", size);
			size = 0;
			printf("\n"ANSI_COLOR_RESET);
		}	
	}
	if(!fileCount) {
		printf(ANSI_COLOR_RED"No files on virtual disk.\n"ANSI_COLOR_RESET);
	}
}

void listBlocks (struct VirtualDisk* disk) {
	unsigned int i,emptyInodes;
	fseek(disk->file,0,SEEK_SET);
	printf(ANSI_COLOR_WHITE "Superblock\t\t" ANSI_COLOR_CYAN "Address: 0\tSize: %ld\n\n", sizeof(struct SuperBlock));
	for(i=0; i < disk->inodesAmount; ++i) {
		printf(ANSI_COLOR_WHITE"Inode %d\t\t"ANSI_COLOR_CYAN, i);
		if(i < 10) printf("\t");
		printf("Address: %ld\t",sizeof(struct SuperBlock)+i*sizeof(struct iNode));
		printf("Size: %ld\t",sizeof(struct iNode));
			if(disk->inodes[i].isUsed == 1) {
				printf("File name: %s\t\t",disk->inodes[i].name);
				if(disk->inodes[i].next == -1) {
					printf("Last node of file\t");
				}
				else {
					printf("Next node: %d\t",disk->inodes[i].next);
				}
			}
			else {
				emptyInodes++;
				printf(ANSI_COLOR_RED"No data"ANSI_COLOR_CYAN);
			}
		printf("\n");
		printf(ANSI_COLOR_WHITE" \\_Block %d\t\t"ANSI_COLOR_CYAN, i);
		printf("Address: %ld\t",sizeof(struct SuperBlock)+disk->inodesAmount*sizeof(struct iNode)+i*BLOCK_SIZE);
		printf("Size: %d\t",BLOCK_SIZE);
		if(disk->inodes[i].isUsed==1) {
			printf("Data size: %d\t",disk->inodes[i].size);
		}
		else {
			printf(ANSI_COLOR_RED"No data"ANSI_COLOR_CYAN);
		}
		printf("\n\n");
	}
	printf(ANSI_COLOR_RESET);
}
