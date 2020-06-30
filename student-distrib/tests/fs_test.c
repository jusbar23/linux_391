#include "../drivers/fs.h"
#include "../lib/lib.h"
#include "tests_files.h"

void 
test_file()
{
	dentry_t entry;
	uint8_t buf[1000];
	
	read_dentry_by_name("frame0.txt", &entry);
	read_data(entry.inode_index, 0, buf, 1000);
	printf("%s\n", buf);

	// read_dentry_by_name("frame1.txt", &entry);
	// read_data(entry.inode_index, 0, buf, 1000);
	// printf("%s\n", buf);

}

void 
test_ls()
{

	dentry_t * dentry_;
	dentry_t test;
	dentry_ = &test;
	uint8_t  buf[6000];

	int8_t fname[32] = {'l','s'};
	read_dentry_by_name (fname, dentry_);
	int ino, count;
	for(count = 0; count < 16; count++){
		read_dentry_by_index (count,dentry_);
		int byte_read = 0;

		for (ino = 0; ino < 10; ino++) 
			byte_read += read_data (dentry_->inode_index, byte_read ,buf, 6000);
		printf("%d: %s: %d bytes \n", count, dentry_->filename,byte_read);
	}
	read_dentry_by_index (1,dentry_);
	return;
}

void 
test_non_existant_file()
{
	dentry_t entry;
	if (read_dentry_by_name("Nonexistantfile.txt", &entry))
	{
		printf("File doesn't exist\n");
		return;
	}
	printf("File Exists (Failed)");
	return;
}
