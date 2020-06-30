#ifndef _FS_H_
#define _FS_H_

#include "../lib/lib.h"
#include "../lib/types.h"
#include "../multiboot.h"
#include "../kernel/pcb.h"

#define FOUR_KB 0x1000

#define RTC_TYPE 0
#define DIRECTORY_TYPE 1
#define FILE_TYPE 2

#define EXEC_MAGIC_0 0x7f
#define EXEC_MAGIC_1 0x45
#define EXEC_MAGIC_2 0x4c
#define EXEC_MAGIC_3 0x46
#define EXEC_MAGIC_NUM_COUNT 4

#define EIGHT_BITS_IN_A_BYTE 8 // Well technically its true

#define ENTRY_POINT_OFFSET 24
#define ENTRY_POINT_SIZE 4

#define USER_PROGRAM_VIRTUAL_START 0x8000000
#define USER_PROGRAM_START 0x800000 // 8 MB
#define USER_PROGRAM_OFFSET 0x048000
#define FOUR_MB 0x400000 // 4 MB


//all necessary structs for the filesystem

typedef struct{
	char filename[32];				//magic number
	uint32_t filetype;
	uint32_t inode_index;
	unsigned char reserved[24];				//magic number				
} __attribute__((packed)) dentry_t; //makesure it's 64 bit

typedef struct{
	uint32_t dir_entry_num;
	uint32_t inode_num;
	uint32_t data_block_num;
	unsigned char reserved[52];				//magic number
	dentry_t dentry_directory[63];			//magic number
} __attribute__((packed)) boot_block_t; //makesure it's next to each other


typedef struct{
	uint8_t byte[FOUR_KB];  //magic number
}__attribute__((packed)) data_block_t;


typedef struct{
	uint32_t length;
	uint32_t data[1023];	//magic number				
} __attribute__((packed)) inode_t;

extern int32_t fs_init(module_t *mod);

// File operations
extern int file_open(const uint8_t *filename);
extern int file_write(uint32_t inode, void* buf, uint32_t bytes);
extern int file_read(uint32_t fd, void* buf, uint32_t bytes);
extern int file_close(int32_t fd, void *buf, uint32_t bytes);

// Directory operations
extern int dir_read(int file_desc, void* buf, uint32_t bytes);
extern int dir_open(const uint8_t *dir_name);
extern int dir_write(int file_desc, void* buf, uint32_t bytes);
extern int dir_close();

// File reading operations
extern int32_t read_dentry_by_name(const int8_t * fname, dentry_t * dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// Load an executable into the correct memory location
extern int32_t loader(const int8_t * filename);
extern uint8_t executable_check(dentry_t * dentry);

#endif /* _FS_H_ */
