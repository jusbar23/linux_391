#include "fs.h"

static boot_block_t *boot_block;
static uint32_t fs_end;
static uint32_t num_directories;
static uint32_t num_inodes;

extern int (*rtc_driver[4]);
extern int (*file_driver[4]);
extern int (*dir_driver[4]);
extern int (*terminal_driver[4]);

/*
 * int32_t fs_init(module_t * mod)
 *   DESCRIPTION: This function initializes the file system, sets up the pointers and information relating to it inside the file system file
 *	 INPUTS: A pointer to the module that holds the file system information	  
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: file system pointers and numbers associated with the file system
 */

int32_t 
fs_init(module_t *mod)
{
	// Mod start passed from kernal.c and points to boot block
	boot_block = (boot_block_t*) mod->mod_start;
	fs_end = mod->mod_end;
	num_directories = boot_block->dir_entry_num;
	num_inodes = boot_block->inode_num;

	printf("Boot Block loaded at address: 0x%#x\n", (unsigned int)boot_block);
	printf("iNodes: %d\n", boot_block->inode_num);
	printf("Dir Entries: %d\n", boot_block->dir_entry_num);
	printf("Data Blocks: %d\n", boot_block->data_block_num);
	return 0;
}

/*
 * int32_t read_dentry_by_name (const int8_t* fname, dentry_t * dentry)
 *   DESCRIPTION: This function reads a file
 *	 INPUTS: fname - the name of the file
 			 dentry - the directory entry	  
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *					-1 on failure
 *   SIDE EFFECTS: none
 */

int32_t 
read_dentry_by_name (const int8_t* fname, dentry_t * dentry)
{

	int currFile;

	//check for dentry null
	if(dentry == NULL) return -1;

	//Transversing the the inode block for 0 to 63
	for(currFile = 0; currFile < num_directories; currFile++) {
		//checking the length of the string using strlen
		if(strlen(fname) != strlen(boot_block->dentry_directory[currFile].filename)) continue;
		
		//comparing the name of the 'file'
		//if it's the same, the copy three thisng

		if(strncmp(fname, boot_block->dentry_directory[currFile].filename, strlen(fname)) == 0) {
			//copy the whole needed bytes for the dentry
			memcpy(dentry, &(boot_block->dentry_directory[currFile]), sizeof(dentry_t));
			return 0;
		}
	}
	// Filename not found
	return -1;
}

/*
 * int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry)
 *   DESCRIPTION: This function reads a file based on an offset
 *	 INPUTS: index - the index from the dir 
 			 dentry - the directory entry to copy into  
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *					-1 on failure
 *   SIDE EFFECTS: none
 */

int32_t 
read_dentry_by_index(uint32_t index, dentry_t * dentry)
{

	if (index < num_directories && index >= 0 && dentry != NULL) {
		//should ask which index is passed in
		memcpy(dentry, &(boot_block->dentry_directory[index]), sizeof(dentry_t));
		return 0;
	}
	else { 
		return -1;
	}
}

/*
 * int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 *   DESCRIPTION: This file reads the data from a file
 *	 INPUTS: inode - the inode offset for the file 
 			 offset - offset into a file to start reading
 			 buf - buffer to copy it into
 			 length - number of bytes to copy  
 *   OUTPUTS: none
 *   RETURN VALUE: # of bytes copied on success
 *					-1 on failure
 *   SIDE EFFECTS: none
 */

int32_t 
read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{

	uint32_t data_block_addr, file_length, i, bytes_in_block_read;
	inode_t *inode_addr;
	data_block_t *current_block;

	if(inode < 0 || buf == NULL || length < 0 || offset < 0)  // checking the parameter
		return -1;

	inode_addr = (inode_t *) (((uint32_t) boot_block) + (inode + 1) * FOUR_KB); 						  // getting the address of the specific inode
	data_block_addr = ((uint32_t) boot_block) + (((boot_block->inode_num) + 1) * FOUR_KB);// getting the first address of the data blocks

	file_length = inode_addr->length; // How many bytes of data are in this file
	current_block = (data_block_t *) (data_block_addr + (inode_addr->data[offset/FOUR_KB] * FOUR_KB)); // Grab the addr of the first data_block

	bytes_in_block_read = 0;

	if (offset > file_length) return 0;

	// iterate through the entire length to read
	for (i = 0; i < length; i++) {
		buf[i] = current_block->byte[offset % FOUR_KB];
		// printf("Byte: 0x%#x, offset: %d\n", buf[i], offset);

		if (++offset % FOUR_KB == 0) {
			current_block = (data_block_t *) (data_block_addr + (inode_addr->data[offset/FOUR_KB] * FOUR_KB));
		}
		
		if (offset > file_length) break;

		bytes_in_block_read++;
	}

	return bytes_in_block_read;
}

/*
 * int file_open(const uint8_t *filename)
 *   DESCRIPTION: Open a file, provides an interface for the driver
 *	 INPUTS: filename - the name of the file to open 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: none
 */

int 
file_open(const uint8_t *filename)
{
	dentry_t dentry;
	pcb_t * curr_pcb = pcb_process();
	void * file_op_tbl;
	int fd;

	if (read_dentry_by_name((int8_t *)filename, &dentry)) return -1;

	//Check if the file type is a file, if it is then
	//set the inode index accordingly, otherwise set it to -1 (null)

	if ((fd = pcb_open(curr_pcb, file_op_tbl)) == -1) return -1;

	switch (dentry.filetype) {
		case FILE_TYPE:
			curr_pcb->elements[fd].file_operation_jmp_tbl = (void *) file_driver;
			break;
		case DIRECTORY_TYPE:
			curr_pcb->elements[fd].file_operation_jmp_tbl = (void *) dir_driver;
			break;
		case RTC_TYPE:
			curr_pcb->elements[fd].file_operation_jmp_tbl = (void *) rtc_driver;
			curr_pcb->rtc = 1;
			break;
	}
	
	if (dentry.filetype == FILE_TYPE) curr_pcb->elements[fd].inode_ptr = dentry.inode_index;

	return fd;
}

/*
 * int file_write(uint32_t inode, void* buf, uint32_t bytes)
 *   DESCRIPTION: Writes to a file from the buffer into the specified inode offset
 *	 INPUTS: inode - offset in the inodes
 *			 buf - buffer to copy from
 * 			 bytes - number of bytes to copy 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *				   -1 on failure
 *   SIDE EFFECTS: none
 */

int 
file_write(uint32_t inode, void* buf, uint32_t bytes)
{
	// uint32_t data_block_addr;
	// dentry_t * dentry_;
	// dentry_t test2;
	// dentry_ = &test2;
	// inode_t* inode_addr,inode_addr2;
	
	// printf("Test to write a file\n");
	// dentry_t* test;
	// num_files++;
	// num_directories++;
	// test=&boot_block->dentry_directory[16];
	
	// test->filename[0]='t';
	// test->filename[1]='e';
	// test->filename[2]='s';
	// test->filename[3]='t';
	// test->filename[4]='.';
	// test->filename[5]='t';
	// test->filename[6]='x';
	// test->filename[7]='t';

	// test->filetype=2;
	// test->inode_index=boot_block->inode_num;
	// boot_block->dir_entry_num++;
	// num_files = boot_block->dir_entry_num;
	

	// uint32_t temp_pointer;
	// data_block_addr = ((uint32_t) boot_block) + (((boot_block->inode_num) + 1) * FOUR_KB);
	// int i;
	// for(i=boot_block->data_block_num;i>0;i--){
	// 	temp_pointer=data_block_addr+((i-1)*FOUR_KB);
	// 	memcpy((data_block_t*)temp_pointer+FOUR_KB,(data_block_t*) temp_pointer, FOUR_KB);
	// }

	// inode_t* insert=(inode_t*)data_block_addr;
	// insert->length=174;
	// insert->data[0]=0;


	return -1;
}

/*
 * int file_read(uint32_t inode, uint32_t offset, void* buf, uint32_t bytes)
 *   DESCRIPTION: Read the data in a file
 *	 INPUTS: inode - offset in the inodes
 *			 offset - offset to start reading
 *			 buf - buffer to read into
 * 			 bytes - number of bytes to read 
 *   OUTPUTS: none
 *   RETURN VALUE: # of bytes read on success
 *				   -1 on failure
 *   SIDE EFFECTS: none
 */

int 
file_read(uint32_t fd, void* buf, uint32_t bytes)
{
	pcb_t * curr_pcb;
	file_descriptor_element_t * file;
	int32_t bytes_read;

    curr_pcb = pcb_process();
    file = &curr_pcb->elements[fd];

    // file is not in use and fails
    // file is directory or RTC then fails
    if (!file->flags) return -1;

	bytes_read = read_data((uint32_t) file->inode_ptr, file->file_position, buf, bytes);
	file->file_position += bytes_read;
	return bytes_read;
}

/*
 * int file_close()
 *   DESCRIPTION: Provides an interface to close a file
 *	 INPUTS: None 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: none
 */

int 
file_close(int32_t fd, void *buf, uint32_t bytes)
{
	pcb_t *curr_pcb;

	if (fd >= 8 || fd < 0) return -1;

	return pcb_close(curr_pcb, fd);
}

/*
 * int dir_open(const uint8_t *dir_name)
 *   DESCRIPTION: Open a directory, provides an interface for the driver
 *	 INPUTS: dir_name - the name of the directory to open 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: none
 */

int
dir_open(const uint8_t *dir_name)
{
	return 0;
}

/*
 * int dir_read(int file_desc, void* buf, uint32_t bytes)
 *   DESCRIPTION: Read the data in a directory
 *	 INPUTS: file_desc - offset in the PCB
 *			 buf - buffer to read into
 * 			 bytes - number of bytes to read 
 *   OUTPUTS: none
 *   RETURN VALUE: # of bytes read on success
 *				   -1 on failure
 *   SIDE EFFECTS: none
 */

int 
dir_read(int file_desc, void* buf, uint32_t bytes)
{
	

	// return byte_read+1;
	
	dentry_t * dentry_;
	dentry_t test;
	dentry_ = &test;
	//currentPCB
	pcb_t* curr_pcb= pcb_process();

	if(curr_pcb->elements[file_desc].file_position>=num_directories) return 0;

	read_dentry_by_index (curr_pcb->elements[file_desc].file_position,dentry_);

	strcpy(buf, dentry_->filename);
	curr_pcb->elements[file_desc].file_position++;
	return strlen(buf);

}

int
dir_write(int file_desc, void* buf, uint32_t bytes) {
	return -1;
}

/*
 * int dir_close()
 *   DESCRIPTION: Provides an interface to close a directory
 *	 INPUTS: None 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: none
 */

int
dir_close() {
	return 0;
}

/*
 * int32_t loader(const int8_t * filename)
 *   DESCRIPTION: Loads a user level program into the appropriate place in memory (128MB + 48KB offset)
 *	 INPUTS: filename - name of the file 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */

int32_t
loader(const int8_t * filename) {
	dentry_t dentry;
	uint8_t entry_point[4];
	uint32_t ret_val = 0;
	int i;

	uint32_t program_start_ptr = USER_PROGRAM_VIRTUAL_START + USER_PROGRAM_OFFSET;
	
	// Check if its a valid file
	if (read_dentry_by_name(filename, &dentry)) return -1;

	// read_data(dentry.inode_index, 0, data, 7000);
	// Loads program
	read_data(dentry.inode_index, 0, (uint8_t *) program_start_ptr, FOUR_MB - USER_PROGRAM_OFFSET);
	
	// Grabs entry point
	read_data(dentry.inode_index, ENTRY_POINT_OFFSET, entry_point, ENTRY_POINT_SIZE);

	for (i = 0; i < ENTRY_POINT_SIZE; i++) {
		ret_val |= entry_point[i] << (i * EIGHT_BITS_IN_A_BYTE);
	}

	return ret_val;
}

/*
 * uint8_t executable_check(dentry_t * dentry)
 *   DESCRIPTION: Checks if a user level program is a valid executable
 *	 INPUTS: entry - entry to read from the file system
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on valid, -1 on invalid
 *   SIDE EFFECTS: none
 */

uint8_t
executable_check(dentry_t * dentry) {
	unsigned char magic_buf[4];

	// Read the four magic numbers in the beginning to check if it's an executable
	if (!read_data(dentry->inode_index, 0, magic_buf, EXEC_MAGIC_NUM_COUNT)) return 0;

	return !(magic_buf[0] == EXEC_MAGIC_0 &&
			magic_buf[1] == EXEC_MAGIC_1 &&
			magic_buf[2] == EXEC_MAGIC_2 &&
			magic_buf[3] == EXEC_MAGIC_3);
}
