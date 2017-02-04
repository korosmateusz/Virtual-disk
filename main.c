#include "header.h"

int main(int argc,char **argv) {
	DISK_NAME = "VirtualDisk";
	char* command;
	struct VirtualDisk* disk;
	size_t size;
	int result;
	if(argc<2) {
		printf(ANSI_COLOR_RED "Wrong use of program.\n"ANSI_COLOR_RESET);
		return 1;
	}
	command=argv[1];
	if(strcmp(command,"create") == 0 && argc==3) {
		size=atoi(argv[2]);
		size *= 1024;
		disk=createDisk(size);
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be created.\n"ANSI_COLOR_RESET );
			return 1;
		}
		else printf("Successfully created virtual disk.\n");
		closeDisk(disk);
	}
	else if(strcmp(command,"delete") == 0 && argc == 2) {
		deleteDisk();
		printf("Successfully deleted virtual disk\n");
	}
	else if(strcmp(command,"listBlocks") == 0 && argc == 2) {
		disk = openDisk();
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be opened.\n"ANSI_COLOR_RESET);
			return 1;
		}
		listBlocks(disk);
		closeDisk(disk);
	}
	else if(strcmp(command,"removeFile") == 0 && argc == 3) {
		disk = openDisk();
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be opened.\n"ANSI_COLOR_RESET);
			return 1;
		}
		result = removeFile(disk,argv[2]);
		if(result == 1) 
			printf(ANSI_COLOR_RED "Failed to remove file.\n"ANSI_COLOR_RESET);
		else
			printf("Remove successful.\n");
		closeDisk(disk);
	}
	else if(strcmp(command,"listFiles") == 0 && argc == 2) {
		disk = openDisk();
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be opened.\n"ANSI_COLOR_RESET);
			return 1;
		}
		listFiles(disk);
		closeDisk(disk);
	}
	else if(strcmp(command,"addFile") == 0 && argc == 4) {
		disk = openDisk();
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be opened.\n" ANSI_COLOR_RESET );
			return 1;
		}
		result = copyToDisk(disk,argv[2],argv[3]);
		if(result == 0) 
			printf("Successfully added file.\n");
		else
			printf(ANSI_COLOR_RED "Failed to add file.\n"ANSI_COLOR_RESET );
		closeDisk(disk);
	}
	else if(strcmp(command,"takeFile") == 0 && argc == 4) {
		disk = openDisk();
		if(!disk) {
			printf(ANSI_COLOR_RED "Virtual disk couldn't be opened.\n"ANSI_COLOR_RESET);
			return 1;
		}
		result = copyFromDisk(disk,argv[2],argv[3]);
		if(result == 0)
			printf("Successfully copied file.\n");
		else
			printf(ANSI_COLOR_RED "Failed to copy file.\n"ANSI_COLOR_RESET);
	}
	else {
		printf(ANSI_COLOR_RED "Wrong use of program.\n"ANSI_COLOR_RESET);
		return 1;
	}
	return 0;
}
