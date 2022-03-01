#include <system.h>

#define SUPERBLOCK_SECTOR 2

File* ext2_open_file(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int startingSector = p->startingSector;
	int s = startingSector;
	char superblock[1024];
	read_sector(storage, s+SUPERBLOCK_SECTOR, 0, 2, superblock);
	int blockSize = 1024<<(charstouint(superblock[27], superblock[26], superblock[25], superblock[24]));
	int blockGroupDescriptorTableStart;
	if (blockSize == 1024) {
		blockGroupDescriptorTableStart = 2;
	} else if (blockSize > 1024) {
		blockGroupDescriptorTableStart = 1;
	} else {
		return UNSUPPORTED_BLOCK_SIZE;
	}
	int totalBlocks = ucharstouint(superblock[7], superblock[6], superblock[5], superblock[4]);
	int totalBlocksPerGroup = ucharstouint(superblock[35], superblock[34], superblock[33], superblock[32]);
	int totalGroup = totalBlocks/totalBlocksPerGroup;
	int totalEntries = totalGroup; //Total entries of Block Group Descriptor Table
	int rootDirNode = 2;
	int inodesPerGroup = ucharstouint(superblock[43], superblock[42], superblock[41], superblock[40]);
	int rootDirBlockGroup = (rootDirNode-1)/inodesPerGroup;
	int rootDirNodeIndex = (rootDirNode-1)%inodesPerGroup;
	int inodeSize = ucharstouint(superblock[89], superblock[88], superblock[87], superblock[86]);
	int inodeNumber = rootDirNodeIndex;
	char* inode = (char*)malloc(inodeSize);
	int nextToken = 1;
	path = strcombine(path, "/");
	int totalToken = get_total_char_from_string(path, '/');
	int DIRECTORY = 2;
	int FILE = 1;
	int fileType = 0; //Directory or file?
	read_inode_again:;
	int rootDirBlock = (inodeNumber*inodeSize)/blockSize;
	char* block = (char*)malloc(blockSize);
	read_sector(storage, s+((rootDirBlock*blockSize)/512), 0, blockSize/512, block);
	memcpy(inode, (char*)((int)block+inodeNumber*inodeSize), inodeSize);
	uint64_t fileSize = (uint64_t)ucharstouint(inode[7], inode[6], inode[5], inode[4]);
	free(block);
	// Now let's read root directory
	int i = 40;
	int rootDirBlockPtr = ucharstouint(inode[i+3], inode[i+2], inode[i+1], inode[i]);
	char* rootDir = (char*)malloc(blockSize);
	char* realFileName;
	int realFileNameLen;
	char* fileName;
	if (nextToken < totalToken) {
		realFileName = get_file_name(path, &nextToken, '/');
		realFileNameLen = strlen(realFileName);
		fileName = get_fat_file_name(realFileName);
	} else {
		nextToken++;
	}
	int totalDirectBlockRead = 0;
	bool fileFound = false;
	while (rootDirBlockPtr != 0 && totalDirectBlockRead < 12) {
		if (nextToken == totalToken) {
			fileType = FILE;
		} else if (nextToken < totalToken) {
			fileType = DIRECTORY;
		}
		read_sector(storage, rootDirBlockPtr, 0, blockSize/512, rootDir);
		if (nextToken == totalToken+1) {
			char* data = rootDir;
			File* file = new File(data, (uint32_t)fileSize);
			return file;
		}
		int j = 0;
		while (rootDir[j] != 0) {
			int entrySize = ucharstoushort(rootDir[5], rootDir[4]);
			int nameLen = uchartouint(rootDir[6]);
			int typeIndicator = uchartouint(rootDir[7]);
			if (typeIndicator == fileType) {
				if (strcmp((char*)((int)rootDir+j+8), fileName, nameLen)) {
					// File or directory found
					printf("File found!\n");
					fileFound = true;
					inodeNumber = ucharstouint(rootDir[j+3], rootDir[j+2], rootDir[j+1], rootDir[j]);
					free(rootDir);
					goto read_inode_again;
				} else {
					return FILE_DOESNT_EXIST;
				}
			}
			j += entrySize;
		}
		i += 4;
		rootDirBlockPtr = ucharstouint(inode[i+3], inode[i+2], inode[i+1], inode[i]);
		totalDirectBlockRead++;
	}
	if (fileFound == false) {
		return FILE_DOESNT_EXIST;
	}
}

int ext2_format(char* path, char* diskLabel, int bytesPerSector, int sectorsPerCluster) {
	return 0;
}

int ext2_delete_file(char* path) {
	return 0;
}

int ext2_write_file(char* path, char* content, int size) {
	return 0;
}

int ext2_update_file(char* path, char* content, int size) {
	return 0;
}