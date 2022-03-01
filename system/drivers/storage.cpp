#include <system.h>

Storage** storages = NULL;
int maxStorages = 5;
int totalStorage = 0;
char next_partition_label = 'C';
static int MBR_SECTOR = 0;
static int READ_ONE_SECTOR = 1;
static int PARTITION_ENTRY_OFFSET = 446;
static int interface;
int totalPartition = 0;
int fileId = 1;
File** openFiles;
static int maxOpenFiles = 1000;
static int totalOpenFiles = 0;

static void read_sector_using_storage(Storage* storage, int lba0, int lba1, int count, char* buffer) {
	uint32_t* info = (uint32_t*)malloc(sizeof(uint32_t)*6);
	info[0] = lba0;
	info[1] = lba1;
	info[2] = count;
	info[3] = (uint32_t)buffer;
	info[4] = storage->deviceType;
	info[5] = storage->controllerType;
	storage->read_sector(info);
}

void add_storage(Storage* storage) {
	if (storages == NULL) {
		storages = (Storage**)malloc(sizeof(Storage*)*maxStorages);
	}
	if (totalStorage >= maxStorages) {
		maxStorages *= 2;
		Storage** tmp = (Storage**)malloc(sizeof(Storage*)*maxStorages);
		for (int i=0; i<totalStorage; i++) {
			tmp[i] = storages[i];
		}
		free(storages);
		storages = tmp;
	}
	storage->storageNumber = totalStorage;
	storage->maxPartitions = 10;
	storage->partitions = (Partition**)malloc(sizeof(Partition*)*storage->maxPartitions);
	storages[totalStorage] = storage;
	totalStorage++;
}

void add_partition(Storage* storage, Partition* partition) {
	if (storage->totalPartition >= storage->maxPartitions) {
		storage->maxPartitions *= 2;
		Partition** tmp = (Partition**)malloc(sizeof(Partition*)*storage->maxPartitions);
		for (int i=0; i<storage->totalPartition; i++) {
			tmp[i] = storage->partitions[i];
		}
		free(storage->partitions);
		storage->partitions = tmp;
	}
	storage->partitions[storage->totalPartition] = partition;
	storage->totalPartition++;
}

Storage* getStorage(char label) {
	for (int i=0; i<totalStorage; i++) {
		for (int j=0; j<storages[i]->totalPartition; j++) {
			if (storages[i]->partitions[j]->label == label) {
				return storages[i];
			}
		}
	}
	return NULL;
}

Partition* getPartition(char label) {
	for (int i=0; i<totalStorage; i++) {
		for (int j=0; j<storages[i]->totalPartition; j++) {
			if (storages[i]->partitions[j]->label == label) {
				return storages[i]->partitions[j];
			}
		}
	}
	return NULL;
}

void read_sector(Storage* storage, int lba0, int lba1, int count, char* buffer) {
	if (interface == IDE) {
		ide_read_sector(storage, lba0, lba1, count, buffer);
	}
}

void write_sector(Storage* storage, int lba0, int lba1, int count, char* buffer) {
	if (interface == IDE) {
		ide_write_sector(storage, lba0, lba1, count, buffer);
	}
}

char get_next_partition_label() {
	char label = next_partition_label;
	next_partition_label++;
	return label;
}

void set_storage_interface(int iface) {
	interface = iface;
}

char* get_fat_file_name(char* _fileName) {
	// Check if file name has extension
	char* fileName = strclone(_fileName);
	int extensionIndex = get_last_char_index_from_string(fileName, '.');
	if (extensionIndex == -1) {
		// This file has no extension
		// Check if the file name length is greater than 8
		int fileNameLen = strlen(fileName);
		if (fileNameLen > 8) {
			// This file has name length greater than 8
			fileName[6] = '~';
			fileName[7] = '1';
			memset(fileName+9, ' ', 4);
			fileName[12] = 0;
		} else {
			memset(fileName+fileNameLen, ' ', 12-fileNameLen);
			fileName[12] = 0;
		}
	} else {
		// This file has extension
		// Check if the file name length is greater than 8
		int fileNameLen = extensionIndex;
		int extensionLength = strlen(fileName)-(extensionIndex+1);
		char* extension = (char*)malloc(extensionLength+1);
		memcpy(extension, (char*)((int)fileName+extensionIndex+1), extensionLength);
		extension[extensionLength] = 0;
		char* newFileName = (char*)malloc(13);
		if (fileNameLen > 8) {
			memcpy(newFileName, fileName, 8);
			newFileName[6] = '~';
			newFileName[7] = '1';
			newFileName[8] = '.';
		} else {
			memcpy(newFileName, fileName, fileNameLen);
			for (int i=fileNameLen; i<8; i++) {
				newFileName[i] = ' ';
			}
		}
		if (extensionLength <= 3) {
			for (int i=0; i<extensionLength; i++) {
				newFileName[8+i] = extension[i];
			}
		} else {
			for (int i=0; i<3; i++) {
				newFileName[8+i] = extension[i];
			}
		}
		newFileName[12] = 0;
		fileName = newFileName;
	}
	// Capitalize all characters
	int i = 0;
	while (fileName[i] != 0) {
		if (fileName[i] >= 'a' && fileName[i] <= 'z') {
			fileName[i] -= 0x20;
		}
		i++;
	}
	return fileName;
}

char* get_file_name(char* path, int* nextToken, char separator) {
	// nextToken is always 1 or greater
	// assumes 'path' is ended with separator
	int i = 0;
	int j = 0;
	while (path[i] != 0) {
		if (path[i] == separator) {
			j++;
		}
		if (j == *nextToken) {
			int nextSeparatorIndex = get_char_index_from_string(path, '/', i+1, 0);
			int fileNameLength = nextSeparatorIndex-i-1;
			char* fileName = (char*)malloc(fileNameLength+1);
			memcpy(fileName, (char*)((int)path+i+1), fileNameLength);
			fileName[fileNameLength] = 0;
			nextToken[0]++;
			return fileName;
		}
		i++;
	}
}

void add_open_file(File* file) {
	if (totalOpenFiles >= maxOpenFiles) {
		maxOpenFiles *= 2;
		File** tmp = (File**)malloc(sizeof(File*)*maxOpenFiles);
		for (int i=0; i<totalOpenFiles; i++) {
			tmp[i] = openFiles[i];
		}
		free(openFiles);
		openFiles = tmp;
	}
	openFiles[totalOpenFiles] = file;
	totalOpenFiles++;
}

File* open_file(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	File* file;
	if (p->fs == FAT32) {
		file = fat32_open_file(path);
	} else if (p->fs == FAT16) {
		file = fat16_open_file(path);
	} else if (p->fs == FAT12) {
		file = fat12_open_file(path);
	} else if (p->fs == EXT2) {
		file = ext2_open_file(path);
	} else {
		return NULL;
	}
	file->path = path;
	file->id = fileId;
	file->readWritePos = 0;
	fileId++;
	add_open_file(file);
	return file;
}

int write_file(char* path, char* content, int size) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	if (p->fs == FAT32) {
		return fat32_write_file(path, content, size);
	} else if (p->fs == FAT16) {
		return fat16_write_file(path, content, size);
	} else if (p->fs == FAT16) {
		return fat12_write_file(path, content, size);
	} else if (p->fs == EXT2) {
		return ext2_write_file(path, content, size);
	} else {
		return UNKNOWN_FILE_SYSTEM;
	}
}

int delete_file(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	if (p->fs == FAT32) {
		return fat32_delete_file(path);
	} else if (p->fs == FAT16) {
		return fat16_delete_file(path);
	} else if (p->fs == FAT12) {
		return fat12_delete_file(path);
	} else if (p->fs == EXT2) {
		return ext2_delete_file(path);
	} else {
		return UNKNOWN_FILE_SYSTEM;
	}
}

int update_file(char* path, char* content, int size) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	if (p->fs == FAT32) {
		return fat32_update_file(path, content, size);
	} else if (p->fs == FAT16) {
		return fat16_update_file(path, content, size);
	} else if (p->fs == FAT12) {
		return fat12_update_file(path, content, size);
	} else if (p->fs == EXT2) {
		return ext2_update_file(path, content, size);
	} else {
		return UNKNOWN_FILE_SYSTEM;
	}
}

int make_directory(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	if (p->fs == FAT32) {
		return fat32_write_file(path, 0, 0);
	} else {
		return UNKNOWN_FILE_SYSTEM;
	}
}

File* get_open_file(int id) {
	for (int i=0; i<totalOpenFiles; i++) {
		if (openFiles[i]->id == id) {
			return openFiles[i];
		}
	}
	return NULL;
}

void seek_file(int fileId, int ptr, int direction, int* endSeekPtr) {
	File* file = get_open_file(fileId);
	if (direction == SEEK_SET) {
		file->readWritePos = ptr;
		endSeekPtr[0] = file->readWritePos;
	} else if (direction == SEEK_END) {
		file->readWritePos = file->size-ptr;
		endSeekPtr[0] = file->readWritePos;
	} else if (direction == SEEK_CUR) {
		file->readWritePos = file->readWritePos+ptr;
		endSeekPtr[0] = file->readWritePos;
	}
}

void read_file(int fileId, char* buffer, int len) {
	File* file = get_open_file(fileId);
	if (file == NULL) {
		return;
	}
	memcpy(buffer, (char*)((int)file->content+file->readWritePos), 1024);
	dump(buffer, 4453);
	printf("Done\n");
	while (1);
	file->readWritePos += len;
}

void init_storages() {
	openFiles = (File**)malloc(sizeof(File*)*maxOpenFiles);
	for (int i=0; i<totalStorage; i++) {
		Storage* storage = storages[i];
		set_storage_interface(storage->interface);
		char mbr[512];
		memset(mbr, 0, 512);
		read_sector(storage, 0, 0, 1, mbr);
		for (int j=0; j<4; j++) {
			int partitionEntryOffset = 446+j*16;
			// Check if Starting sector value is 0, if so, then continue
			/*if ((mbr[partitionEntryOffset+2]&0x3F)==0) {
				continue;
			}*/
			// Check if SystemID byte value is 0, if so, then continue
			unsigned int systemId = uchartouint(mbr[partitionEntryOffset+4]);
			if (systemId == 0) {
				continue;
			}
			// Check if Relative Sector value is 0, if so, then continue
			int relativeSector = ucharstouint(mbr[partitionEntryOffset+11], mbr[partitionEntryOffset+10], mbr[partitionEntryOffset+9], mbr[partitionEntryOffset+8]);
			if (relativeSector == 0) {
				continue;
			}
			// Check if Total Sector value is 0, if so, then continue
			int totalSector = (int)mbr[partitionEntryOffset+15]*0x1000000+(int)mbr[partitionEntryOffset+14]*0x10000+(int)mbr[partitionEntryOffset+13]*0x100+(int)mbr[partitionEntryOffset+12];
			if (totalSector == 0) {
				continue;
			}
			char vbr[512];
			read_sector(storage, relativeSector, 0, 1, vbr);
			if (systemId == 0x0B || systemId == 0x06 || systemId == 0x01 || systemId == 0x83) {
				Partition* p = (Partition*)malloc(sizeof(Partition));
				p->storageNumber = storage->storageNumber;
				p->partitionInfoLBA = 0; //Partition info (starting LBA, etc) is stored in the MBR (sector 0)
				p->partitionInfoOffset = partitionEntryOffset;
				p->label = get_next_partition_label();
				p->startingSector = relativeSector;
				int bytesPerSector = ucharstoushort(vbr[12], vbr[11]);
				int totalSectors = ucharstoushort(vbr[20], vbr[19]);
				if (totalSectors == 0) {
					totalSectors = ucharstouint(vbr[35], vbr[34], vbr[33], vbr[32]);
				}
				p->bytesPerSector = bytesPerSector;
				p->totalSectors = totalSectors;
				if (systemId == 0x0B) {
					p->fs = FAT32;
				} else if (systemId == 0x06) {
					p->fs = FAT16;
				} else if (systemId == 0x01) {
					p->fs = FAT12;
				} else if (systemId == 0x83) {
					p->fs = EXT2;
				}
				p->type = storage->type;
				add_partition(storage, p);
				totalPartition++;
			}
		}
	}
}

int format_partition(char* path, char* diskLabel, int bytesPerSector, int sectorsPerCluster) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	if (p->fs == FAT32) {
		return fat32_format(path, diskLabel, bytesPerSector, sectorsPerCluster);
	} else if (p->fs == FAT16) {
		return fat16_format(path, diskLabel, bytesPerSector, sectorsPerCluster);
	} else if (p->fs == FAT12) {
		return fat12_format(path, diskLabel, bytesPerSector, sectorsPerCluster);
	} else {
		return UNKNOWN_FILE_SYSTEM;
	}
}
