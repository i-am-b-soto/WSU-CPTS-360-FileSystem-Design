#include "include.h"

OFT *getOFT(){
	int i;

	for (i = 0; i < NOFT; i++){
		OFT *current = &oft[i];
		if (current->refCount == 0){ //found the one we need
			return current;
		}	
	}

	return 0;
}

int my_open(char *pathname){

	//1. ask for a pathname and mode to open:
	char file_mode[16];
	printf("Enter a mode to open: \n");
	fgets(file_mode, 16, stdin);
	file_mode[strlen(file_mode)-1] = 0;

	//2. get pathname's inumber:

	int mode = atoi(file_mode);
	int file_dev = root->dev;
	if (*pathname != '/'){
		file_dev = running->cwd->dev;
	}

	int ino = getino(&dev, pathname);

	MINODE *mip = iget(dev, ino); //3. get its Minode pointer

	//4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.

	//5. allocate a FREE OpenFileTable (OFT) and fill in values:

	OFT *oftp = getOFT();
		
	if (oftp == 0){
		printf("oftp is null\n");
		return;
	}

	oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	//oftp->minodePtr = mip;  // point at the file's minode[]
}
