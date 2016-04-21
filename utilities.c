#include "include.h"

extern int bdealloc(int dev, int bno);
extern int idealloc(int dev, int ino);

int iput(MINODE *mip)
{
	printf("entering iput\n");
	if (!mip) { 
		printf("mip is null\n");
		return;	
	}
	mip->refCount--; //decrease refCount by 1
	if (mip->refCount > 0){
		printf("Case 1 in iput\n");
		return 0;
	}
	strcpy(mip->name, "");
	if (mip->dirty == 0){
		printf("Case 2 in iput\n");
		return 0;
	}
	if (mip->dirty == 1){
		printf("Case 3 in iput\n");
		//must write the INODE back to disk
		int blk    = (mip->ino - 1) / 8 + bg_inode_table;
		int offset = (mip->ino - 1) % 8;
		get_block(fd, blk, buf);

		ip = (INODE *)buf + offset;
		*ip = mip->INODE;

		put_block(fd, blk, buf);
	}
}

int tst_bit(char *buf, int bit)
{
	int i, j;
	i = bit/8; j=bit%8;
	if (buf[i] & (1 << j))
		return 1;
	return 0;
}

int set_bit(char *buf, int bit)
{
	int i, j;
	i = bit/8; j=bit%8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
	int i, j;
	i = bit/8; j=bit%8;
	buf[i] &= ~(1 << j);
}	

int ialloc(int dev)
{
	int  i;
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, imap, buf);

	printf("NINODES = %d\n", ninodes);
	getchar();

	for (i=0; i < ninodes; i++){
		if (tst_bit(buf, i)==0){
			set_bit(buf,i);
			decFreeInodes(dev);
			put_block(dev, imap, buf);
			return i+1;
		}
	}
	printf("ialloc(): no more free inodes\n");
	return 0;
}

int balloc(int dev){

	char buf[1024];
	get_block(dev, bmap, buf);

	int i;

	for (i = 0; i < nblocks; i++){
		if (tst_bit(buf,i) == 0){
			set_bit(buf,i);	
			decFreeBlocks(dev);
			put_block(dev, bmap, buf);
			printf("balloc(): bno = %d\n", i+1);
			return i+1;
		}
	}
	printf("balloc(): no more data blocks\n");
}

int bdealloc(int dev, int bno){
	char buf[1024];
	get_block(dev, bno, buf); //get the block we need
	memset(buf, 0, 1024);	//set it all to 0's
	put_block(dev, bno, buf); //write the 0's back
	
	//reset bmap
}

int idealloc(int dev, int ino){
	
	char buf[1024];
	int blk = (ino - 1) / 8 + bg_inode_table;
	int offset = (ino - 1) % 8;
	get_block(dev, blk, buf);
	INODE *found_ino = (INODE*)buf + offset;

	//clear its data blocks

	memset(buf, 0, 1024);

	int i;
	for (i = 0; i < EXT2_N_BLOCKS; i++){
		if (found_ino->i_block[i] == 0){
			continue;
		}
		//we need to deallocate
		put_block(dev, found_ino->i_block[i], buf);
	}
	
	//deallocate found_ino

	get_block(dev, blk, buf);
	found_ino = (INODE*)buf + offset;
	found_ino = 0;
	put_block(dev, blk, buf);

	//get the imap

	get_block(fd, imap, buf);
	set_bit(buf, ino-1); //clear the imap at ino	
	put_block(fd, imap, buf);  //write back the new imap
}

int decFreeBlocks(int dev){
	//decrement free blocks in super and gd
	
	char buf[BLKSIZE];

	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count--;
	put_block(dev, 1, buf);
	
	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_blocks_count--;
	put_block(dev, 2, buf);

	nblocks--;
}

int decFreeInodes(int dev)
{
	char buf[BLKSIZE];

	// dec free inodes count in SUPER and GD
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

int findmyname(MINODE *parent, int myino, char *myname) 
{
	char *parent_name = parent->name;
	INODE parent_inode = parent->INODE;return;
	/*
	   Given the parent DIR (MINODE pointer) and myino, this function finds 
	   the name string of myino in the parent's data block. This is the SAME
	   as SEARCH() by myino, then copy its name string into myname[ ].
	*/
	int i = 0;
	
	char *cp;  char temp[256];
       	DIR  *dp;
	for (i = 0; i < 12; i++){
		int current_block = parent_inode.i_block[i];
		get_block(fd, ip->i_block[i], buf);     // read INODE's i_block[0]
	       	cp = buf;  
	       	dp = (DIR*)buf;
	       	while(cp < buf + BLKSIZE){
	      		if (dp->rec_len == 0) { return;}// printf("Could not find %s\n", folder_name); }
			if (dp->inode == myino){
				strncpy(temp, dp->name, dp->name_len);
				temp[dp->name_len] = 0;
				strcpy(myname, temp);
				return 0;
			}	      		
			// move to the next DIR entry:
	      		cp += (dp->rec_len);   // advance cp by rec_len BYTEs
	      		dp = (DIR*)cp;     // pull dp along to the next record
	       	}
	}
}

int print_dir_entries(MINODE *mip)
{
	char temp_name[256];
	strcpy(temp_name, mip->name); 
	printf("Printing dir entries for %s\n", temp_name);
	int i; 
	char *cp, sbuf[BLKSIZE];
	DIR *dp;
	INODE *ip = &(mip->INODE);

	for (i=0; i<12; i++){  // ASSUME DIRs only has 12 direct blocks
		if (ip->i_block[i] == 0)
		{
			if (i == 0)
				printf("Returning because block is 0\n");
			return 0;
		}

		//get ip->i_block[i] into sbuf[ ];
		printf("Printing block %d\n", ip->i_block[i]);
		get_block(fd, (ip->i_block[i]), sbuf);	
		//sbuf = ip->i_block[i];
		dp = (DIR *)sbuf;
		cp = sbuf;
		while (cp < sbuf + BLKSIZE){			  	
			printf("inode=%.4d rec_len=%.4d name_len=%.4d name=%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
			if (dp->rec_len == 0) return;
			// print dp->inode, dp->rec_len, dp->name_len, dp->name);
		  	// WRITE YOUR CODE TO search for name: return its ino if found
			//int name_ino_num = search_inode(ip, "asdads");
			
			//int blk = (name_ino_num - 1) / 8 + bg_inode_table;
			//int offset = (name_ino_num - 1) % 8; 			
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
	}
	return 0;
}

int findino(MINODE *mip, int *myino, int *parentino)
{
	/*
	For a DIR Minode, extract the inumbers of . and .. 
	Read in 0th data block. The inumbers are in the first two dir entries.
	*/	
}

int search(INODE * inodePtr, char * directory_name)
{
	//get the data block of inodePtr
	printf("===================================\n");
	int k, i;
	char *cp;  char temp[256];
       	DIR  *dp;

       	// ASSUME INODE *ip -> INODE
	for (i = 0; i < 12; i++)
	{

	       	//printf("i_block[%d] = %d\n", i, inodePtr->i_block[i]); // print blk number
	       	get_block(fd, inodePtr->i_block[i], buf);     // read INODE's i_block[0]
	       	cp = buf;  
	       	dp = (DIR*)buf;
	       	while(cp < buf + BLKSIZE){
			printf("Searching for %s\n", directory_name);
			//getchar();	      		
			strncpy(temp, dp->name, dp->name_len);
	      		temp[dp->name_len] = 0;
	      		//printf("%.4d  %.4d  %.4d  [%s]\n", dp->inode, dp->rec_len, dp->name_len, temp);
			//printf("comparing [%s] and [%s]\n", temp, directory_name);
			if (strcmp(directory_name, temp) == 0)
			{
				//printf("Found %s\n", directory_name);

				return dp->inode;
			}else if (dp->rec_len == 0) { break; }
	      		//getchar();			
	      		// move to the next DIR entry:
	      		cp += (dp->rec_len);   // advance cp by rec_len BYTEs
	      		dp = (DIR*)cp;     // pull dp along to the next record
	       	}
	}

	return 0;
}

char *parse_pathname(char *pathname){
	static int location = 0;
	if (location >= strlen(pathname)) return 0;

	int i = location;
	char found = 0;

	char piece[256];

	while (pathname[i]){
		if (pathname[i] == '/'){
			i++; //increment i to make it point after the end
			found = 1;
			break;
		}
		else{
			piece[i - location] = pathname[i]; //copy 1 byte
		}
		i++;
	}
	
	if (!(pathname[i])) i++;
	piece[i - location - 1] = 0;
	location = i;
	
	char *piece_pointer = piece;

	return piece_pointer;
}



/*

MINODE *iget(int dev, int ino)
{
	
	//(1). Search minode[i] for an entry whose refCount > 0 with the SAME (dev,ino)
     	//     if found: refCount++; mip-return &minode[i];
	//(2). Find a minode[i] whose refCount = 0 => let MINODE *mip = &minode[i];
	

	MINODE *mip = 0;
	int i;
	for (i = 0; i < NMINODES; i++)
	{
		if (minode[i].dev == dev && minode[i].ino == ino){
			minode[i].refCount++;
			return &minode[i];
		}else if (minode[i].refCount == 0 && mip == 0){
			mip = &(minode[i]);
			printf("Found minode at index %d\n", i);		
		}	
	}

      //(3). Use Mailman's algorithm to compute

	printf("bg_inode_table = %d\n", bg_inode_table);

	int blk = (ino - 1) / 8 + bg_inode_table;
	int offset = (ino - 1) % 8;

    //  (4). read blk into buf[ ]; 	

	printf("Getting block %d with offset %d from file descriptor %d\n", blk, offset, fd);
	get_block(fd, blk, buf);
	INODE *ip = (INODE *)buf;
	ip += offset;

      //(5). COPY *ip into mip->INODE

	mip->INODE = *ip;

     // (6). initialize other fields of *mip: 

	mip->ino      = ino; 
	mip->dev      = dev;

	mip->refCount =   1;
	mip->dirty    =   0;
	mip->mounted  =   0;
	mip->mountptr =   0;

	return mip;
}

*/

int search_minode(MINODE *mip, char *directory_name)
{
	int i;
	for (i = 0; i < NMINODES; i++){
		MINODE current = minode[i];
		if (strcmp(current.name, directory_name) == 0){
			return current.ino;	
		}
	}
	return -1;
}

/**
 * Takes a inode and its number and returns the parent MINODE pointer.
 * Is there a more effecient way of doing this?
 */
MINODE *getParentNode(INODE * ip, int inum){
	char *cp;  char temp[256];
    DIR  *dp;
    INODE *tempNode;

	int i =0;

	if(inum < 2){
		printf("Encountered unknown inode %d\n", inum);
		return 0;
	}
	// Search the given inodes' i_blocks
	for (i = 0; i < 12; i++)
	{
	       	// Convert the i_block[0] to a buff
	       	get_block(fd, ip->i_block[i], buf);
	       	cp = buf;
	       	// Convert the buff to a DIR
	       	dp = (DIR*)buf;

	       	while(cp < buf + BLOCK_SIZE){
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			
			printf("searching in getParentIno %s\n", temp);

			if (dp->rec_len == 0) { 
				printf("encountered a record length of 0 for [%s]\n", temp); 
				return 0;
			}

			// We have found the parent DIR
			if (strcmp(dp->name, "..") == 0 )
			{
				// Convert the inumber to a MINODE, be sure to put this away
				MINODE* mip = iget(dev,dp->inode);
				return mip;
			}
			// move to the next DIR entry:
				cp += (dp->rec_len);   // advance cp by rec_len BYTEs
				dp = (DIR*)cp;     // pull dp along to the next record
		}

	}
	return 0;
}

DIR * getDir(INODE * ip, int inum){
	char *cp;  char temp[256];
    DIR  *dp;
    INODE *tempNode;


	int i =0;
	// Convert the inumber to a MINODE, be sure to put this away
	MINODE* mip;
	mip= getParentNode(ip, inum);
	if (mip == 0) {
		printf("Error traversing file system \n");
		return 0;
	}

	tempNode = &(mip->INODE);


    get_block(fd, tempNode->i_block[i], buf);
    cp = buf;
    dp = (DIR*)buf;

    while(cp < buf + BLOCK_SIZE){
    	strncpy(temp, dp->name, dp->name_len);
    	temp[dp->name_len] = 0;
    	if (dp->rec_len == 0) {
    		iput(mip);
			return 0; }

    	if(dp->inode == inum ){
		//printf("Found the return dir: %s\n", dp->name );
		iput(mip);
		return dp;
		      		}
		// move to the next DIR entry:
		cp += (dp->rec_len);   // advance cp by rec_len BYTEs
		dp = (DIR*)cp;
	}

}



MINODE *iget(int dev, int ino)
{
	char temp_name[256];
	printf("Entering iget\n");
	/*
	(1). Search minode[i] for an entry whose refCount > 0 with the SAME (dev,ino)
     	     if found: refCount++; mip-return &minode[i];
	(2). Find a minode[i] whose refCount = 0 => let MINODE *mip = &minode[i];
	*/

	MINODE *mip = 0;
	int i;
	for (i = 0; i < NMINODES; i++)
	{
		if (minode[i].dev == dev && minode[i].ino == ino){
			// If the minode doesn't have a name...
			if(strcmp(minode[i].name, "") == 0){
				// If ino isn't root..
				if(ino > 2) {
					// Assign a name to the bitch.
					INODE *ptr = &(minode[i].INODE);
					DIR * dp  = (DIR*)getDir(ptr, minode[i].ino );
					if(dp != 0){
						strncpy(temp_name, dp->name, dp->name_len);
						temp_name[dp->name_len] = 0;
						printf("The directory namelength is: %d\n", dp->name_len);
						printf("Adding a mip name %s, \n", temp_name);
						strcpy(minode[i].name, temp_name);
						//minode[i].name[dp->name_len] =0;
					}
				}
			}

			printf("Found a minode with name: %s and index: %d\n", minode[i].name, i);
			minode[i].refCount++;
			return &minode[i];
		}else if (minode[i].refCount == 0 && mip == 0){
			mip = &(minode[i]);
			printf("\ninode allocated at index %d\n", i);
		}	
	}

      /*(3). Use Mailman's algorithm to compute*/

	printf("bg_inode_table = %d\n", bg_inode_table);

	int blk = (ino - 1) / 8 + bg_inode_table;
	int offset = (ino - 1) % 8;

    /*  (4). read blk into buf[ ]; 	*/

	printf("Getting block %d with offset %d from file descriptor %d with ino_num = %d\n", blk, offset, fd, ino);
	get_block(fd, blk, buf);
	INODE *ip = (INODE *)buf + offset;
	
      //(5). COPY *ip into mip->INODE

	mip->INODE = *ip;


	// We need the dir to obtain the name!
	// But let's not find the name of root, that causes errors...
	if(ino > 2) {
		// Assign a name to this bitch.
		DIR * dp  = getDir(ip, ino);

		if(dp != 0){
			strncpy(temp_name, dp->name, dp->name_len);
			printf("The directory namelength is: %d\n", dp->name_len);
			temp_name[dp->name_len] = 0;
			printf("Adding a mip name %s, \n", temp_name);
			strcpy(mip->name, temp_name);
		}
	}
     // (6). initialize other fields of *mip:

	mip->ino      = ino; 
	mip->dev      = dev;

	mip->refCount =   1;
	mip->dirty    =   0;
	mip->mounted  =   0;
	mip->mountptr =   0;

	printf("exiting iget\n");
	
	return mip;
}


/**
 *   Takes an inode and its inumber and returns its corresponding
 *   dir pointer
 */
















