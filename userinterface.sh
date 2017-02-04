#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

gcc -c functions.c
gcc -c main.c
gcc functions.o main.o -o DiskManagement
chmod +x DiskManagement

if [ ! -f VirtualDisk ]
then
	read -p "What size(in kilobytes) should virtual disk have? " size
	./DiskManagement create $size
fi

if [ ! -f VirtualDisk ]
then
	echo -e ${RED}Failed to create a virtual disk ${NC}
	exit 1
fi

while true
do
	read -p "What would you like to do with the virtual disk? (delete, addFile, takeFile, removeFile, listFiles, listBlocks, exit) " answer
	case $answer in
	delete ) 
		./DiskManagement delete
		break;;
	addFile ) 
		read -p "What is the name of the file you wish to copy? " filename
		read -p "What should the file be called on the virtual disk? " diskfilename
		./DiskManagement addFile $filename $diskfilename
		;;
	takeFile ) 
		read -p "What is the name of the file you wish to copy? " diskfilename
		read -p "What should the file be called on the drive? " filename
		./DiskManagement takeFile $diskfilename $filename
		;;
	removeFile ) 
		read -p "What is the name of the file you wish to remove? " diskfilename
		./DiskManagement removeFile $diskfilename
		;;
	listFiles )
		./DiskManagement listFiles
		;;
	listBlocks )
		./DiskManagement listBlocks
		;;
	exit )
		exit 0
		;;
	* ) echo -e ${RED}Please specify a proper action ${NC}
		;;
	esac
done
