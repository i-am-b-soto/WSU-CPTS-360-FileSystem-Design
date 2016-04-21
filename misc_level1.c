#include "include.h"

char nthBit(int value, int bit){
	return (value & (1 << bit)) >> bit;
}

int my_chmod(){

	char buf[1024];
	printf("pathname = %s parameter = %s\n", pathname, parameter);
	int mode = atoi(pathname);

	if (mode == 0){
		printf("%s is not a new valid mode\n", pathname);
	}

	//are we relative to the root or relative to cwd?
	INODE *current = &root->INODE;
	if (*pathname != '/'){
		current = &running->cwd->INODE;
		//pathname++;
	}

	char *piece;
	int blk;
	while( piece = parse_pathname(parameter) ){
		printf("Current piece is %s\n", piece);
		int ino = search(current, piece);
		if (ino == 0){
			printf("Could not find %s\n", piece);
		}
		blk = (ino - 1) / 8 + bg_inode_table;
		int offset = (ino - 1) % 8;
		//get the next inode
		get_block(fd, blk, buf);
		current = (INODE *)buf + offset;
	}
	//change the permission of the current inode and write back to block
	//i_mode is the variable to change
	
	//777 % 10 = 7

	//get the mode
	short new_mode = current->i_mode;

	//get the three parts
	char user  = mode % 10;
	char group = (mode / 10) % 10;
	char world = (mode / 100) % 10;

	char permission_bits[9];
	permission_bits[0] = nthBit(user, 2); 
	permission_bits[1] = nthBit(user, 1); 
	permission_bits[2] = nthBit(user, 0);
	permission_bits[3] = nthBit(group, 2); 
	permission_bits[4] = nthBit(group, 1); 
	permission_bits[5] = nthBit(group, 0);
	permission_bits[6] = nthBit(world, 2); 
	permission_bits[7] = nthBit(world, 1); 
	permission_bits[8] = nthBit(world, 0);
	
	int i;
	for (i = 0; i < 9; i++){
		if (i != 0 && i % 3 == 0){
			putchar(' ');
		}
		printf("%d", permission_bits[i]);	
		//set the 7th + ith bit
		int bit_mask = ~(permission_bits[i] << (7 + i));
		new_mode &= bit_mask;
	}
	current->i_mode = new_mode;
	putchar('\n');

	printf("Printing new mode = \n");

	for (i = 0; i < 9; i++){
		printf("%d", nthBit(current->i_mode, abs(6 - i)));
		//if(i > 0 && i % 3 == 0) putchar(' ');
	}
	putchar('\n');
}














