#include <system.h>

int fat12_get_time(int hour, int minute, int second) {
	return (hour<<11)|(minute<<5)|(second/2);
}

int fat12_format(char* path, char* diskLabel, int bytesPerSector, int sectorsPerCluster) {
	if (sectorsPerCluster == 0) {
		sectorsPerCluster = 8;
	}
	if (bytesPerSector == 0) {
		bytesPerSector = 512;
	}
	int bytesPerCluster = sectorsPerCluster*(2*bytesPerSector)+2*8;
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int partitionSize = p->bytesPerSector*p->totalSectors;
	int RESERVED_SECTORS = 32;
	int reservedBytes = RESERVED_SECTORS*bytesPerSector;
	partitionSize -= reservedBytes;
	int availableClusters = partitionSize/bytesPerCluster;
	int sectorsPerFat = availableClusters*3/2/bytesPerSector;
	//char* vbr = (char*)&fat12_vbr_template;
	char vbr[512];
	read_sector(storage, p->startingSector, 0, 1, vbr);
	vbr[11] = bytesPerSector&0xFF;
	vbr[12] = bytesPerSector>>8&0xFF;
	vbr[13] = (unsigned char)sectorsPerCluster;
	vbr[14] = RESERVED_SECTORS&0xFF;
	vbr[15] = RESERVED_SECTORS>>8&0xFF;
	vbr[16] = 0x02;
	int directoryEntries = sectorsPerCluster*bytesPerSector/32;
	//vbr[17] = 0;
	//vbr[18] = 0;
	int totalSectors = partitionSize/bytesPerSector;
	if (totalSectors < 65536) {
		vbr[19] = totalSectors&0xFF;
		vbr[20] = totalSectors>>8&0xFF;
	} else {
		vbr[19] = 0;
		vbr[20] = 0;
	}
	if (p->type != FLOPPY) {
		vbr[21] = 0xF8; //Fixed disk
	} else {
		if ((partitionSize/1024)==2880) {
			vbr[21] = 0xF0;
		} else if ((partitionSize/1024)==1440) {
			vbr[21] = 0xF0;
		} else if ((partitionSize/1024)==720) {
			vbr[21] = 0xF9;
		} else if ((partitionSize/1024)==1200) {
			vbr[21] = 0xF9;
		} else if ((partitionSize/1024)==360) {
			vbr[21] = 0xFD;
		} else if ((partitionSize/1024)==320) {
			vbr[21] = 0xFF;
		} else if ((partitionSize/1024)==180) {
			vbr[21] = 0xFC;
		} else if ((partitionSize/1024)==160) {
			vbr[21] = 0xFE;
		}
	}
	vbr[22] = sectorsPerFat&0xFF;
	vbr[23] = sectorsPerFat>>8&0xFF;
	int sectorsPerTrack = 63;
	//vbr[24] = sectorsPerTrack&0xFF;
	//vbr[25] = sectorsPerTrack>>8&0xFF;
	int heads = 0xFF;
	/*vbr[26] = heads&0xFF;
	vbr[27] = heads>>8&0xFF;
	vbr[28] = p->startingSector&0xFF;
	vbr[29] = p->startingSector>>8&0xFF;
	vbr[30] = p->startingSector>>16&0xFF;
	vbr[31] = p->startingSector>>24;*/
	if (totalSectors >= 65536) {
		vbr[32] = totalSectors&0xFF;
		vbr[33] = totalSectors>>8&0xFF;
		vbr[34] = totalSectors>>16&0xFF;
		vbr[35] = totalSectors>>24;
	}
	vbr[22] = sectorsPerFat&0xFF;
	vbr[23] = sectorsPerFat>>8&0xFF;
	if (p->type == FLOPPY) {
		vbr[36] = 0x00;
	} else if (p->type == HARDDISK) {
		vbr[36] = 0x80;
	}
	vbr[38] = 0x29;
	int day = get_day()&0xFF;
	int weekday = get_weekday()&0xFF;
	int month = get_month()&0xFF;
	int year = get_year()&0xFF;
	int serialNumber = (day<<24)|(weekday<<16)|(month<<8)|year;
	/*vbr[39] = serialNumber&0xFF;
	vbr[40] = serialNumber>>8&0xFF;
	vbr[41] = serialNumber>>16&0xFF;
	vbr[42] = serialNumber>>24;*/
	int labelLen = strlen(diskLabel);
	if (labelLen < 11) {
		char* newLabel = (char*)malloc(11);
		memcpy(newLabel, diskLabel, labelLen);
		for (int i=labelLen; i<11; i++) {
			newLabel[i] = ' ';
		}
		diskLabel = newLabel;
	}
	//memcpy((char*)((int)vbr+43), diskLabel, 11);
	//memcpy((char*)((int)vbr+82), "fat12   ", 8);
	vbr[510] = 0x55;
	vbr[511] = 0xAA;
	unsigned char prev_vbr[512];
	read_sector(storage, p->startingSector, 0, 1, prev_vbr);
	int prevReservedSectors = ucharstoushort(prev_vbr[15], prev_vbr[14]);
	int prevFatTableStart = p->startingSector+prevReservedSectors;
	int prevFatSectors = ucharstoushort(prev_vbr[23], prev_vbr[22]);
	int rootDirEntries = ucharstoushort(prev_vbr[18], prev_vbr[17]);
	sectorsPerCluster = uchartouint(prev_vbr[13]);
	bytesPerSector = ucharstoushort(prev_vbr[12], prev_vbr[11]);
	unsigned char fat[512];
	memset(fat, 0, 512);
	// Empty previous FAT table
	write_sector(storage, prevFatTableStart, 0, 1, fat);
	for (int i=0; i<prevFatSectors-1; i++) {
		write_sector(storage, prevFatTableStart+1+i, 0, 1, fat);
	}
	// Empty previous second FAT table
	prevFatTableStart += prevFatSectors;
	write_sector(storage, prevFatTableStart, 0, 1, fat);
	for (int i=0; i<prevFatSectors-1; i++) {
		write_sector(storage, prevFatTableStart+1+i, 0, 1, fat);
	}
	prevFatTableStart = p->startingSector+prevReservedSectors;
	int prevRootDirSector = prevFatTableStart+(2*prevFatSectors);
	printf("Previous root dir sectors: %x\n", prevRootDirSector);
	int prevDataStart = prevRootDirSector+rootDirEntries*32/bytesPerSector;
	// Empry previous root directory
	char* rootDir = (char*)malloc(rootDirEntries*32);
	memset(rootDir, 0, rootDirEntries*32);
	write_sector(storage, prevRootDirSector, 0, rootDirEntries*32/bytesPerSector, rootDir);
	free(rootDir);
	int fatTableStart = p->startingSector+RESERVED_SECTORS;
	int fatSectors = availableClusters*3/2/bytesPerSector;
	// Empty FAT table
	memset16u(fat, 0x0FF8, 1);
	memset16u((unsigned char*)((int)fat+2), 0xFFFF, 1);
	memset16u((unsigned char*)((int)fat+4), 0x0FFF, 1);
	write_sector(storage, fatTableStart, 0, 1, fat);
	memset(fat, 0, 512);
	for (int i=0; i<fatSectors-1; i++) {
		write_sector(storage, fatTableStart+1+i, 0, 1, fat);
	}
	// Empty second FAT table
	memset16u(fat, 0x0FF8, 1);
	memset16u((unsigned char*)((int)fat+2), 0xFFFF, 1);
	memset16u((unsigned char*)((int)fat+4), 0x0FFF, 1);
	fatTableStart += fatSectors;
	write_sector(storage, fatTableStart, 0, 1, fat);
	memset(fat, 0, 512);
	for (int i=0; i<fatSectors-1; i++) {
		write_sector(storage, fatTableStart+1+i, 0, 1, fat);
	}
	fatTableStart = p->startingSector+RESERVED_SECTORS;
	// Empty root directory
	rootDir = (char*)malloc(rootDirEntries*32);
	memset(rootDir, 0, rootDirEntries*32);
	int rootStart = p->startingSector+RESERVED_SECTORS+2*fatSectors;
	write_sector(storage, rootStart, 0, sectorsPerCluster, rootDir);
	free(rootDir);
	// Update VBR of the partition
	write_sector(storage, p->startingSector, 0, 1, vbr);
	// Update data in MBR
	char mbr[512];
	memset(mbr, 0, 512);
	read_sector(storage, p->partitionInfoLBA, 0, 1, mbr);
	int offset = p->partitionInfoOffset;
	mbr[offset+4] = 0x06; //fat12
	mbr[offset+12] = totalSectors&0xFF;
	mbr[offset+13] = totalSectors>>8&0xFF;
	mbr[offset+14] = totalSectors>>16&0xFF;
	mbr[offset+15] = totalSectors>>24;
	write_sector(storage, p->partitionInfoLBA, 0, 1, mbr);
	return 0;
}

int fat12_delete_file(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int startingSector = p->startingSector;
	char vbr[512];
	read_sector(storage, startingSector, 0, 1, vbr);
	int bytesPerSector = ucharstoushort(vbr[12], vbr[11]);
	int sectorsPerCluster = uchartouint(vbr[13]);
	int bytesPerCluster = bytesPerSector*sectorsPerCluster;
	int sectorsPerFat = ucharstoushort(vbr[23], vbr[22]);
	int reservedSectors = ucharstoushort(vbr[15], vbr[14]);
	int fatCount = uchartouint(vbr[16]);
	int rootDirStart = p->startingSector+reservedSectors+fatCount*sectorsPerFat;
	int rootDirEntries = ucharstoushort(vbr[18], vbr[17]);
	int rootDirSectors = rootDirEntries*32/bytesPerSector;
	int rootDirBytes = rootDirSectors*bytesPerSector;
	/*printf("Starting sector: %d\n", startingSector);
	printf("Reserved sectors: %d\n", reservedSectors);
	printf("Sectors per FAT: %d\n", sectorsPerFat);
	printf("Starting sector: %d\n", startingSector);
	printf("FAT count: %d\n", fatCount);
	printf("Data offset: %d\n", (startingSector+reservedSectors+fatCount*sectorsPerFat));*/
	char* rootDir = (char*)malloc(rootDirBytes);
	int dataStart = rootDirStart+rootDirSectors;
	int nextToken = 1;
	path = strcombine(path, "/");
	int totalToken = get_total_char_from_string(path, '/');
	int rootDirCluster = rootDirStart;
	read_file_again:;
	char* realFileName = get_file_name(path, &nextToken, '/');
	int realFileNameLen = strlen(realFileName);
	char* fileName = get_fat_file_name(realFileName);
	int fileEntryPos = 0;
	read_root_dir_again:;
	int i = 0;
	bool fileFound = false;
	read_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
	while (rootDir[i] != 0) {
		fileEntryPos = i;
		if (rootDir[i+11] != 0x0F) {
			// Indicates this is not long file name entry
			if (strcmp((char*)((int)rootDir+i), fileName, 11)) {
				// File found
				// Check if long file name has the same name as the original name
				int lfnPos = i-32;
				int fnPos = 0;
				bool fileNameNotSame = false;
				if (rootDir[lfnPos+11] != 0x0F) {
					// This file has no Long File Name entry
					fileFound = true;
					break;
				}
				if (rootDir[lfnPos] > 0x40) {
					for (int j=0; j<5; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<6; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<2; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
				} else {
					while (rootDir[lfnPos] < 0x40) {
						int prevLfnPos = lfnPos;
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<5; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<6; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<2; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						lfnPos = prevLfnPos;
						lfnPos -= 32;
					}
				}
			}
		}
		i += 32;
	}
	if (!fileFound) {
		// Maybe the file is in the next cluster of the root directory
		int fatPos = rootDirCluster*3/2/bytesPerSector;
		char fat[bytesPerSector];
		read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
		int fatEntryPos = rootDirCluster*3/2-(fatPos*bytesPerSector);
		unsigned int fatEntry = ucharstoushort(fat[fatEntryPos+1], fat[fatEntryPos]);
		if ((rootDirCluster*3%2)==0) {
			fatEntry &= 0x0FFF;
		} else {
			fatEntry >>= 8;
		}
		if (fatEntry >= 0x0FF8) {
			// File is not found in the folder
			return NULL;
		} else {
			int clusterNumber = fatEntry;
			rootDirCluster = (clusterNumber-2)*sectorsPerCluster+dataStart;
			goto read_root_dir_again;
		}
	}
	if (fileFound) {
		printf("File found!\n");
		i = fileEntryPos;
		int highCluster = ucharstoushort(rootDir[i+21], rootDir[i+20]);
		int lowCluster = ucharstoushort(rootDir[i+27], rootDir[i+26]);
		int clusterNumber = (highCluster<<16)|lowCluster;
		int attrib = rootDir[i+11];
		if ((attrib&0x10) != 0) {
			// This is a directory
			// If you want to delete directory?
			if (nextToken == totalToken) {
				// Then delete that
				goto delete_file;
			}
			// Open directory
			rootDirCluster = (clusterNumber-2)*sectorsPerCluster+dataStart;
			goto read_file_again;
		}
		delete_file:;
		rootDir[i] = 0xE5; //Deleted or emptied
		// Flush directory
		write_sector(storage, rootDirCluster, 0, sectorsPerCluster, rootDir);
		return 0;
	}
}

int fat12_write_file(char* path, char* content, int size) {
	bool makeDirectory = false;
	// If content NULL and size is 0, then you want to make directory
	if (content == NULL && size == 0) {
		makeDirectory = true;
	}
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int startingSector = p->startingSector;
	char vbr[512];
	read_sector(storage, startingSector, 0, 1, vbr);
	int bytesPerSector = (int)vbr[12]*0x100+(int)vbr[11];
	int sectorsPerCluster = (int)vbr[13];
	int bytesPerCluster = bytesPerSector*sectorsPerCluster;
	int sectorsPerFat = ucharstoushort(vbr[23], vbr[22]);
	int reservedSectors = (int)vbr[15]*0x100+(int)vbr[14];
	int fatCount = (int)vbr[16];
	int rootDirStart = p->startingSector+reservedSectors+fatCount*sectorsPerFat;
	int rootDirEntries = ucharstoushort(vbr[18], vbr[17]);
	int rootDirSectors = rootDirEntries*32/bytesPerSector;
	int rootDirBytes = rootDirSectors*bytesPerSector;
	int dataStart = rootDirStart+rootDirSectors;
	/*printf("Starting sector: %d\n", startingSector);
	printf("Reserved sectors: %d\n", reservedSectors);
	printf("Sectors per FAT: %d\n", sectorsPerFat);
	printf("Starting sector: %d\n", startingSector);
	printf("FAT count: %d\n", fatCount);
	printf("Data offset: %d\n", (startingSector+reservedSectors+fatCount*sectorsPerFat));*/
	char* rootDir = (char*)malloc(rootDirBytes);
	//int sector = (9-2)*sectorsPerCluster+dataStart;
	//printf("%x\n", sector);
	//stop();
	int nextToken = 1;
	path = strcombine(path, "/");
	int totalToken = get_total_char_from_string(path, '/');
	int rootDirCluster = rootDirStart;
	read_file_again:;
	char* realFileName = get_file_name(path, &nextToken, '/');
	int realFileNameLen = strlen(realFileName);
	char* fileName = get_fat_file_name(realFileName);
	int fileEntryPos = 0;
	read_root_dir_again:;
	int i = 0;
	bool fileFound = false;
	read_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
	while (rootDir[i] != 0) {
		fileEntryPos = i;
		if (rootDir[i+11] != 0x0F) {
			// Indicates this is not long file name entry
			if (strcmp((char*)((int)rootDir+i), fileName, 11)) {
				// File found
				// Check if long file name has the same name as the original name
				int lfnPos = i-32;
				int fnPos = 0;
				bool fileNameNotSame = false;
				if (rootDir[lfnPos+11] != 0x0F) {
					// This file has no Long File Name entry
					fileFound = true;
					break;
				}
				if (rootDir[lfnPos] > 0x40) {
					for (int j=0; j<5; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<6; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<2; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
				} else {
					while (rootDir[lfnPos] < 0x40) {
						int prevLfnPos = lfnPos;
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<5; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<6; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<2; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						lfnPos = prevLfnPos;
						lfnPos -= 32;
					}
				}
			}
		}
		i += 32;
	}
	if (fileFound) {
		if (nextToken == totalToken) {
			// Return 'cause the file is already exists
			return FILE_ALREADY_EXISTS;
		} else {
			// This is a folder
			i = fileEntryPos;
			int highCluster = (int)rootDir[i+21]*0x100+(int)rootDir[i+20];
			int lowCluster = (int)rootDir[i+27]*0x100+(int)rootDir[i+26];
			int clusterNumber = (highCluster<<16)|lowCluster;
			int attrib = rootDir[i+11];
			if ((attrib&0x10) != 0) {
				// This is a directory
				rootDirCluster = (clusterNumber-2)*sectorsPerCluster+dataStart;
				goto read_file_again;
			}
			return FILE_ALREADY_EXISTS;
		}
	} else {
		if (nextToken == totalToken) {
			if (makeDirectory) {
				// If you want to make directory
				
			}
			// The file is not yet been written, so write it based on the content
			char directoryEntry[32];
			memcpy(directoryEntry, fileName, 11);
			int hour = cmos_get_hour();
			int minute = cmos_get_minute();
			int second = cmos_get_second();
			int creationTime = ((hour&0x1F)<<11)|((minute&0x3F)<<5)|(second&0x1F);
			directoryEntry[14] = creationTime&0xFF;
			directoryEntry[15] = creationTime>>8&0xFF;
			int day = cmos_get_day();
			int month = cmos_get_month();
			int year = cmos_get_year();
			int creationDate = ((year&0x7F)<<11)|((month&0x0F)<<5)|(day&0x1F);
			creationDate = 0;
			directoryEntry[16] = creationDate&0xFF;
			directoryEntry[17] = creationDate>>8&0xFF;
			int accessDate = creationDate;
			accessDate = 0;
			directoryEntry[18] = accessDate&0xFF;
			directoryEntry[19] = accessDate>>8&0xFF;
			int modificationTime = creationTime;
			modificationTime = 0;
			directoryEntry[22] = modificationTime&0xFF;
			directoryEntry[23] = modificationTime>>8&0xFF;
			int modificationDate = creationDate;
			modificationDate = 0;
			directoryEntry[24] = modificationDate&0xFF;
			directoryEntry[25] = modificationDate>>8&0xFF;
			if (makeDirectory) {
				//Directory (0x10) or Archive (0x20)?
				directoryEntry[11] = 0x10;
			} else {
				directoryEntry[11] = 0x20;
			}
			if (!makeDirectory) {
				directoryEntry[28] = size&0xFF;
				directoryEntry[29] = size>>8&0xFF;
				directoryEntry[30] = size>>16&0xFF;
				directoryEntry[31] = size>>24&0xFF;
			}
			// Find a free cluster in FAT table
			int fatPos = 0;
			int fatEntryPos = 0;
			bool fatEntryFound = false;
			unsigned char fat[512];
			int allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
			int fatOffset;
			for (int j=0; j<allFatSectors; j++) {
				read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				fatOffset = fatEntryPos-(fatPos*512);
				for (int k=0; k<512; k++) {
					int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
					if (fatEntry == 0x0000) {
						// Found free entry
						fatEntryFound = true;
						goto fat_entry_found;
					}
					fatEntryPos += 2;
					fatOffset += 2;
				}
				fatPos++;
			}
			fat_entry_found:;
			if (!fatEntryFound) {
				return FULL_PARTITION;
			}
			int freeCluster = fatEntryPos*3/2;
			printf("Free directory cluster: %d\n", freeCluster);
			printf("Make directory? %b\n", makeDirectory);
			directoryEntry[20] = freeCluster>>16&0xFF;
			directoryEntry[21] = freeCluster>>24;
			directoryEntry[26] = freeCluster&0xFF;
			directoryEntry[27] = freeCluster>>8&0xFF;
			// Update directory entry of the file
			memcpy((char*)((int)rootDir+i), directoryEntry, 32);
			write_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
			if (makeDirectory) {
				fat[fatOffset] = 0xFF;
				fat[fatOffset+1] = 0x0F;
				write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				// Empty directory
				int dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
				char directory[bytesPerCluster];
				memset(directory, 0, bytesPerCluster);
				write_sector(storage, dataSector, 0, sectorsPerCluster, directory);
				return 0;
			}
			int neededClusters = size/bytesPerCluster+((size%bytesPerCluster)?1:0);
			int dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
			for (int j=0; j<neededClusters; j++) {
				printf("Writing cluster...\n");
				// Write data at freecluster
				write_sector(storage, dataSector, 0, sectorsPerCluster, content);
				// Update fat table
				if (j == neededClusters-1) {
					// This is last cluster in chain, so write 0x0FFFFFFF in FAT table entry
					fat[fatOffset] = 0xFF;
					fat[fatOffset+1] = 0x0F;
					write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
					break;
				}
				int prevFatOffset = fatOffset;
				int prevFatPos = fatPos;
				int prevFreeCluster = freeCluster;
				// Find next free cluster for the next cluster
				printf("FAT position: %d\n", fatPos);
				fatPos = 0;
				fatEntryPos = 0;
				fatEntryFound = false;
				allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
				fatOffset = 0;
				for (int j=0; j<allFatSectors; j++) {
					read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
					fatOffset = fatEntryPos-(fatPos*512);
					for (int k=0; k<512; k++) {
						int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
						if (fatOffset != prevFatOffset) {
							if (fatEntry == 0x0000) {
								// Found free entry
								fatEntryFound = true;
								goto free_entry_found;
							}
						}
						fatEntryPos += 2;
						fatOffset += 2;
					}
					fatPos++;
				}
				free_entry_found:;
				if (fatEntryFound == false) {
					return FULL_PARTITION;
				}
				freeCluster = fatEntryPos*3/2;
				fat[prevFatOffset] = freeCluster&0xFF;
				fat[prevFatOffset+1] = freeCluster>>8&0xFF;
				fat[prevFatOffset+2] = freeCluster>>16&0xFF;
				fat[prevFatOffset+3] = freeCluster>>24;
				write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
				content = (char*)((int)content+j*bytesPerCluster);
			}
		} else {
			// The folder is not found, so create it
			// Find for free cluster
			int fatPos = 0;
			int fatEntryPos = 0;
			bool fatEntryFound = false;
			char fat[512];
			int allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
			int fatOffset;
			for (int j=0; j<allFatSectors; j++) {
				read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				fatOffset = fatEntryPos-(fatPos*512);
				for (int k=0; k<512; k++) {
					int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
					if (fatEntry == 0x0000) {
						// Found free entry
						fatEntryFound = true;
						goto free_fat_entry_found;
					}
					fatEntryPos += 2;
					fatOffset += 2;
				}
				fatPos++;
			}
			free_fat_entry_found:;
			if (!fatEntryFound) {
				return FULL_PARTITION;
			}
			int freeCluster = fatEntryPos*3/2;
			printf("[Create folder] Free cluster: %d\n", freeCluster);
			int directorySector = (freeCluster-2)*sectorsPerCluster+dataStart;
			char directory[32];
			memset(directory, 0, 32);
			memcpy(directory, fileName, 11);
			directory[11] = 0x08; //Is a Volume ID (folder name)
			int hour = cmos_get_hour();
			int minute = cmos_get_minute();
			int second = cmos_get_second();
			int modificationTime = ((hour&0x1F)<<11)|((minute&0x3F)<<5)|(second&0x1F);
			directory[22] = modificationTime&0xFF;
			directory[23] = modificationTime>>8&0xFF;
			int day = cmos_get_day();
			int month = cmos_get_month();
			int year = cmos_get_year();
			int modificationDate = ((year&0x7F)<<11)|((month&0x0F)<<5)|(day&0x1F);
			directory[24] = modificationDate&0xFF;
			directory[25] = modificationDate>>8&0xFF;
			// Flush new directory
			write_sector(storage, directorySector, 0, 1, directory);
			// Update FAT table
			fat[fatOffset] = 0xFF;
			fat[fatOffset+1] = 0x0F;
			write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
			// Update previous directory with new entry for new directory
			char rootDirEntry[32];
			memset(rootDirEntry, 0, 32);
			memcpy(rootDirEntry, fileName, 11);
			rootDirEntry[11] = 0x10; //Directory (0x10) or Archive (0x20)?
			int creationTime = modificationTime;
			rootDirEntry[14] = creationTime&0xFF;
			rootDirEntry[15] = creationTime>>8&0xFF;
			int creationDate = modificationDate;
			rootDirEntry[16] = creationDate&0xFF;
			rootDirEntry[17] = creationDate>>8&0xFF;
			int accessDate = creationDate;
			rootDirEntry[18] = accessDate&0xFF;
			rootDirEntry[19] = accessDate>>8&0xFF;
			rootDirEntry[20] = freeCluster>>16&0xFF;
			rootDirEntry[21] = freeCluster>>24;
			rootDirEntry[22] = modificationTime&0xFF;
			rootDirEntry[23] = modificationTime>>8&0xFF;
			rootDirEntry[24] = modificationDate&0xFF;
			rootDirEntry[25] = modificationDate>>8&0xFF;
			rootDirEntry[26] = freeCluster&0xFF;
			rootDirEntry[27] = freeCluster>>8&0xFF;
			memcpy((char*)((int)rootDir+i), rootDirEntry, 32);
			write_sector(storage, rootDirCluster, 0, sectorsPerCluster, rootDir);
			rootDirCluster = (freeCluster-2)*sectorsPerCluster+dataStart;
			goto read_file_again;
		}
	}
	return 0;
}

int fat12_update_file(char* path, char* content, int size) {
	bool makeDirectory = false;
	// If content NULL and size is 0, then you want to make directory
	if (content == NULL && size == 0) {
		makeDirectory = true;
	}
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int startingSector = p->startingSector;
	char vbr[512];
	read_sector(storage, startingSector, 0, 1, vbr);
	int bytesPerSector = ucharstoushort(vbr[12], vbr[11]);
	int sectorsPerCluster = uchartouint(vbr[13]);
	int bytesPerCluster = bytesPerSector*sectorsPerCluster;
	int sectorsPerFat = ucharstoushort(vbr[23], vbr[22]);
	int reservedSectors = ucharstoushort(vbr[15], vbr[14]);
	int fatCount = uchartouint(vbr[16]);
	int rootDirStart = p->startingSector+reservedSectors+fatCount*sectorsPerFat;
	int rootDirEntries = ucharstoushort(vbr[18], vbr[17]);
	int rootDirSectors = rootDirEntries*32/bytesPerSector;
	int rootDirBytes = rootDirSectors*bytesPerSector;
	int dataStart = rootDirStart+rootDirSectors;
	/*printf("Starting sector: %d\n", startingSector);
	printf("Reserved sectors: %d\n", reservedSectors);
	printf("Sectors per FAT: %d\n", sectorsPerFat);
	printf("Starting sector: %d\n", startingSector);
	printf("FAT count: %d\n", fatCount);
	printf("Data offset: %d\n", (startingSector+reservedSectors+fatCount*sectorsPerFat));*/
	char* rootDir = (char*)malloc(rootDirBytes);
	//int sector = (9-2)*sectorsPerCluster+dataStart;
	//printf("%x\n", sector);
	//stop();
	int nextToken = 1;
	path = strcombine(path, "/");
	int totalToken = get_total_char_from_string(path, '/');
	int rootDirCluster = rootDirStart;
	read_file_again:;
	char* realFileName = get_file_name(path, &nextToken, '/');
	int realFileNameLen = strlen(realFileName);
	char* fileName = get_fat_file_name(realFileName);
	int fileEntryPos = 0;
	read_root_dir_again:;
	int i = 0;
	bool fileFound = false;
	read_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
	while (rootDir[i] != 0) {
		fileEntryPos = i;
		if (rootDir[i+11] != 0x0F) {
			// Indicates this is not long file name entry
			if (strcmp((char*)((int)rootDir+i), fileName, 11)) {
				// File found
				// Check if long file name has the same name as the original name
				int lfnPos = i-32;
				int fnPos = 0;
				bool fileNameNotSame = false;
				if (rootDir[lfnPos+11] != 0x0F) {
					// This file has no Long File Name entry
					fileFound = true;
					break;
				}
				if (rootDir[lfnPos] > 0x40) {
					for (int j=0; j<5; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<6; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<2; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
				} else {
					while (rootDir[lfnPos] < 0x40) {
						int prevLfnPos = lfnPos;
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<5; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<6; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<2; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						lfnPos = prevLfnPos;
						lfnPos -= 32;
					}
				}
			}
		}
		i += 32;
	}
	if (fileFound) {
		if (nextToken == totalToken) {
			// Update file now. 'Cause file is found.
			int attrib = rootDir[i+11];
			if ((attrib&0x10) != 0) {
				// This is directory. Who would update directory?
				return CANNOT_UPDATE_DIRECTORY;
			}
			i = fileEntryPos;
			int highCluster = ucharstoushort(rootDir[i+21], rootDir[i+20]);
			int lowCluster = ucharstoushort(rootDir[i+27], rootDir[i+26]);
			int clusterNumber = (highCluster<<16)|lowCluster;
			int prevSize = ucharstouint(rootDir[i+31], rootDir[i+30], rootDir[i+29], rootDir[i+28]);
			//printf("Previous size: %d\n", prevSize);
			int newSize = prevSize+size;
			rootDir[i+28] = newSize&0xFF;
			rootDir[i+29] = newSize>>8&0xFF;
			rootDir[i+30] = newSize>>16&0xFF;
			rootDir[i+31] = newSize>>24;
			write_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
			printf("Writing sector...\n");
			int lastCluster;
			// Follow chain until get last cluster
			follow:;
			int fatEntryPos = clusterNumber*3/2;
			//printf("FAT entry position: %d\n", fatEntryPos);
			int fatPos = (fatEntryPos/512);
			char fat[512];
			read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
			int fatOffset = fatEntryPos-(fatPos*512);
			unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
			//printf("FAT entry value: %x\n", fatEntry);
			if (fatEntry >= 0x0FF8) {
				// This is the last cluster in chain
				//printf("Found last entry: %x\n", fatEntry);
				lastCluster = clusterNumber;
				goto found_last_entry;
			}
			clusterNumber = fatEntry;
			goto follow;
			found_last_entry:;
			int retry = 0;
			if ((prevSize%bytesPerCluster)!=0) {
				write_updated_data_again:;
				printf("Writing new data:\n");
				int dataSector = (lastCluster-2)*sectorsPerCluster+dataStart;
				char* tmpData = (char*)malloc(bytesPerCluster);
				read_sector(storage, dataSector, 0, sectorsPerCluster, tmpData);
				int lastOffset = prevSize%bytesPerCluster;
				memcpy((char*)((int)tmpData+lastOffset), content, bytesPerCluster-lastOffset);
				content = (char*)((int)content+(bytesPerCluster-lastOffset));
				size -= (bytesPerCluster-lastOffset);
				write_sector(storage, dataSector, 0, sectorsPerCluster, tmpData);
				free(tmpData);
				if (size > 0) {
					// Find free FAT entry again
					int prevFatPos = fatPos;
					int prevFatOffset = fatOffset;
					char tmpFat[512];
					memcpy(tmpFat, fat, 512);
					fatEntryPos = lastCluster*3/2;
					fatPos = (fatEntryPos/512);
					fatOffset = fatEntryPos-(fatPos*512);
					int allFatSectors = bytesPerSector*sectorsPerFat/512;
					for (int j=0; j<allFatSectors; j++) {
						read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
						for (int k=0; k<512; k++) {
							unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
							if (fatEntry == 0x0000) {
								// Write this new FAT entry to the previous FAT entry
								lastCluster = fatEntryPos*3/2;
								printf("Last cluster: %x\n", lastCluster);
								printf("Previous FAT offset: %d\n", prevFatOffset);
								printf("FAT offset: %d\n", fatOffset);
								tmpFat[fatOffset] = 0xFF;
								tmpFat[fatOffset+1] = 0x0F;
								tmpFat[prevFatOffset] = lastCluster&0xFF;
								tmpFat[prevFatOffset+1] = lastCluster>>8&0xFF;
								write_sector(storage, startingSector+reservedSectors+prevFatPos, 0, 1, tmpFat);
								memcpy(fat, tmpFat, 512);
								prevSize = 0;
								retry++;
								goto write_updated_data_again;
							}
							fatOffset += 2;
							fatEntryPos += 2;
						}
						fatPos++;
					}
					return FULL_PARTITION;
				} else {
					// Find free entry for last cluster
					printf("[Last cluster] %d\n", lastCluster);
					fatEntryPos = lastCluster*3/2;
					fatPos = (fatEntryPos/512);
					fatOffset = fatEntryPos-(fatPos*512);
					printf("[FAT position] %d\n", fatPos);
					int allFatSectors = bytesPerSector*sectorsPerFat/512;
					for (int j=0; j<allFatSectors; j++) {
						read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
						for (int k=0; k<512; k++) {
							unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
							printf("[Last cluster] FAT entry: %d\n", fatEntry);
							if (fatEntry == 0x0000) {
								printf("Last cluster\n");
								printf("FAT offset: %d\n", fatOffset);
								fat[fatOffset] = 0xFF;
								fat[fatOffset+1] = 0x0F;
								write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
								return 0;
							}
							fatOffset += 2;
							fatEntryPos += 2;
						}
					}
					return FULL_PARTITION;
				}
			} else {
				printf("Previous size: %d\n", prevSize);
			}
		} else {
			// This is a folder
			i = fileEntryPos;
			int highCluster = (int)rootDir[i+21]*0x100+(int)rootDir[i+20];
			int lowCluster = (int)rootDir[i+27]*0x100+(int)rootDir[i+26];
			int clusterNumber = (highCluster<<16)|lowCluster;
			int attrib = rootDir[i+11];
			if ((attrib&0x10) != 0) {
				// This is a directory
				rootDirCluster = (clusterNumber-2)*sectorsPerCluster+dataStart;
				goto read_file_again;
			}
			return FILE_ALREADY_EXISTS;
		}
	} else {
		if (nextToken == totalToken) {
			if (makeDirectory) {
				// If you want to make directory
				
			}
			// The file is not yet been written, so write it based on the content
			char directoryEntry[32];
			memcpy(directoryEntry, fileName, 11);
			int hour = cmos_get_hour();
			int minute = cmos_get_minute();
			int second = cmos_get_second();
			int creationTime = ((hour&0x1F)<<11)|((minute&0x3F)<<5)|(second&0x1F);
			directoryEntry[14] = creationTime&0xFF;
			directoryEntry[15] = creationTime>>8&0xFF;
			int day = cmos_get_day();
			int month = cmos_get_month();
			int year = cmos_get_year();
			int creationDate = ((year&0x7F)<<11)|((month&0x0F)<<5)|(day&0x1F);
			creationDate = 0;
			directoryEntry[16] = creationDate&0xFF;
			directoryEntry[17] = creationDate>>8&0xFF;
			int accessDate = creationDate;
			accessDate = 0;
			directoryEntry[18] = accessDate&0xFF;
			directoryEntry[19] = accessDate>>8&0xFF;
			int modificationTime = creationTime;
			modificationTime = 0;
			directoryEntry[22] = modificationTime&0xFF;
			directoryEntry[23] = modificationTime>>8&0xFF;
			int modificationDate = creationDate;
			modificationDate = 0;
			directoryEntry[24] = modificationDate&0xFF;
			directoryEntry[25] = modificationDate>>8&0xFF;
			if (makeDirectory) {
				//Directory (0x10) or Archive (0x20)?
				directoryEntry[11] = 0x10;
			} else {
				directoryEntry[11] = 0x20;
			}
			if (!makeDirectory) {
				directoryEntry[28] = size&0xFF;
				directoryEntry[29] = size>>8&0xFF;
				directoryEntry[30] = size>>16&0xFF;
				directoryEntry[31] = size>>24&0xFF;
			}
			// Find a free cluster in FAT table
			int fatPos = 0;
			int fatEntryPos = 0;
			bool fatEntryFound = false;
			char fat[512];
			int allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
			int fatOffset;
			for (int j=0; j<allFatSectors; j++) {
				read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				fatOffset = fatEntryPos-(fatPos*512);
				for (int k=0; k<512; k++) {
					unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
					if (fatEntry == 0x0000) {
						// Found free entry
						fatEntryFound = true;
						goto fat_entry_found;
					}
					fatEntryPos += 2;
					fatOffset += 2;
				}
				fatPos++;
			}
			fat_entry_found:;
			if (!fatEntryFound) {
				return FULL_PARTITION;
			}
			int freeCluster = fatEntryPos*3/2;
			printf("Free directory cluster: %d\n", freeCluster);
			printf("Make directory? %b\n", makeDirectory);
			directoryEntry[20] = freeCluster>>16&0xFF;
			directoryEntry[21] = freeCluster>>24;
			directoryEntry[26] = freeCluster&0xFF;
			directoryEntry[27] = freeCluster>>8&0xFF;
			// Update directory entry of the file
			memcpy((char*)((int)rootDir+i), directoryEntry, 32);
			write_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
			if (makeDirectory) {
				fat[fatOffset] = 0xFF;
				fat[fatOffset+1] = 0x0F;
				write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				// Empty directory
				int dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
				char directory[bytesPerCluster];
				memset(directory, 0, bytesPerCluster);
				write_sector(storage, dataSector, 0, rootDirSectors, directory);
				return 0;
			}
			int neededClusters = size/bytesPerCluster+((size%bytesPerCluster)?1:0);
			int dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
			for (int j=0; j<neededClusters; j++) {
				printf("Writing cluster...\n");
				// Write data at freecluster
				write_sector(storage, dataSector, 0, sectorsPerCluster, content);
				// Update fat table
				if (j == neededClusters-1) {
					// This is last cluster in chain, so write 0x0FFFFFFF in FAT table entry
					fat[fatOffset] = 0xFF;
					fat[fatOffset+1] = 0x0F;
					write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
					break;
				}
				int prevFatOffset = fatOffset;
				int prevFatPos = fatPos;
				int prevFreeCluster = freeCluster;
				// Find next free cluster for the next cluster
				printf("FAT position: %d\n", fatPos);
				fatPos = 0;
				fatEntryPos = 0;
				fatEntryFound = false;
				allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
				fatOffset = 0;
				for (int j=0; j<allFatSectors; j++) {
					read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
					fatOffset = fatEntryPos-(fatPos*512);
					for (int k=0; k<512; k++) {
						unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
						if (fatOffset != prevFatOffset) {
							if (fatEntry == 0x0000) {
								// Found free entry
								fatEntryFound = true;
								goto free_entry_found;
							}
						}
						fatEntryPos += 2;
						fatOffset += 2;
					}
					fatPos++;
				}
				free_entry_found:;
				if (fatEntryFound == false) {
					return FULL_PARTITION;
				}
				freeCluster = fatEntryPos*3/2;
				fat[prevFatOffset] = freeCluster&0xFF;
				fat[prevFatOffset+1] = freeCluster>>8&0xFF;
				write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				dataSector = (freeCluster-2)*sectorsPerCluster+dataStart;
				content = (char*)((int)content+j*bytesPerCluster);
			}
		} else {
			// The folder is not found, so create it
			// Find for free cluster
			int fatPos = 0;
			int fatEntryPos = 0;
			bool fatEntryFound = false;
			char fat[512];
			int allFatSectors = fatCount*sectorsPerFat*bytesPerSector/512;
			int fatOffset;
			for (int j=0; j<allFatSectors; j++) {
				read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
				fatOffset = fatEntryPos-(fatPos*512);
				for (int k=0; k<512; k++) {
					unsigned int fatEntry = ucharstoushort(fat[fatOffset+1], fat[fatOffset]);
					if (fatEntry == 0x0000) {
						// Found free entry
						fatEntryFound = true;
						goto free_fat_entry_found;
					}
					fatEntryPos += 2;
					fatOffset += 2;
				}
				fatPos++;
			}
			free_fat_entry_found:;
			if (!fatEntryFound) {
				return FULL_PARTITION;
			}
			int freeCluster = fatEntryPos*3/2;
			printf("[Create folder] Free cluster: %d\n", freeCluster);
			int directorySector = (freeCluster-2)*sectorsPerCluster+dataStart;
			char directory[32];
			memset(directory, 0, 32);
			memcpy(directory, fileName, 11);
			directory[11] = 0x08; //Is a Volume ID (folder name)
			int hour = cmos_get_hour();
			int minute = cmos_get_minute();
			int second = cmos_get_second();
			int modificationTime = ((hour&0x1F)<<11)|((minute&0x3F)<<5)|(second&0x1F);
			directory[22] = modificationTime&0xFF;
			directory[23] = modificationTime>>8&0xFF;
			int day = cmos_get_day();
			int month = cmos_get_month();
			int year = cmos_get_year();
			int modificationDate = ((year&0x7F)<<11)|((month&0x0F)<<5)|(day&0x1F);
			directory[24] = modificationDate&0xFF;
			directory[25] = modificationDate>>8&0xFF;
			// Flush new directory
			write_sector(storage, directorySector, 0, 1, directory);
			// Update FAT table
			fat[fatOffset] = 0xFF;
			fat[fatOffset+1] = 0xFF;
			fat[fatOffset+2] = 0xFF;
			fat[fatOffset+3] = 0x0F;
			write_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
			// Update previous directory with new entry for new directory
			char rootDirEntry[32];
			memset(rootDirEntry, 0, 32);
			memcpy(rootDirEntry, fileName, 11);
			rootDirEntry[11] = 0x10; //Directory (0x10) or Archive (0x20)?
			int creationTime = modificationTime;
			rootDirEntry[14] = creationTime&0xFF;
			rootDirEntry[15] = creationTime>>8&0xFF;
			int creationDate = modificationDate;
			rootDirEntry[16] = creationDate&0xFF;
			rootDirEntry[17] = creationDate>>8&0xFF;
			int accessDate = creationDate;
			rootDirEntry[18] = accessDate&0xFF;
			rootDirEntry[19] = accessDate>>8&0xFF;
			rootDirEntry[20] = freeCluster>>16&0xFF;
			rootDirEntry[21] = freeCluster>>24;
			rootDirEntry[22] = modificationTime&0xFF;
			rootDirEntry[23] = modificationTime>>8&0xFF;
			rootDirEntry[24] = modificationDate&0xFF;
			rootDirEntry[25] = modificationDate>>8&0xFF;
			rootDirEntry[26] = freeCluster&0xFF;
			rootDirEntry[27] = freeCluster>>8&0xFF;
			memcpy((char*)((int)rootDir+i), rootDirEntry, 32);
			write_sector(storage, rootDirCluster, 0, rootDirSectors, rootDir);
			rootDirCluster = (freeCluster-2)*sectorsPerCluster+dataStart;
			goto read_file_again;
		}
	}
	return 0;
}

File* fat12_open_file(char* path) {
	char label = path[0];
	Storage* storage = getStorage(label);
	set_storage_interface(storage->interface);
	Partition* p = getPartition(label);
	int startingSector = p->startingSector;
	char vbr[512];
	read_sector(storage, startingSector, 0, 1, vbr);
	int bytesPerSector = (int)vbr[12]*0x100+(int)vbr[11];
	int sectorsPerCluster = (int)vbr[13];
	int bytesPerCluster = bytesPerSector*sectorsPerCluster;
	int reservedSectors = (int)vbr[15]*0x100+(int)vbr[14];
	int rootDirEntries = ucharstoushort(vbr[18], vbr[17]);
	int sectorsPerFat = ucharstoushort(vbr[23], vbr[22]);
	int fatCount = (int)vbr[16];
	int rootDirStart = p->startingSector+reservedSectors+fatCount*sectorsPerFat;
	/*printf("Starting sector: %d\n", startingSector);
	printf("Reserved sectors: %d\n", reservedSectors);
	printf("Sectors per FAT: %d\n", sectorsPerFat);
	printf("Starting sector: %d\n", startingSector);
	printf("FAT count: %d\n", fatCount);
	printf("Data offset: %d\n", (startingSector+reservedSectors+fatCount*sectorsPerFat));*/
	char* rootDir = (char*)malloc(rootDirEntries*32);
	int dataStart = startingSector+reservedSectors+fatCount*sectorsPerFat+rootDirEntries*32/bytesPerSector;
	int nextToken = 1;
	path = strcombine(path, "/");
	int rootDirCluster = rootDirStart;
	read_file_again:;
	char* realFileName = get_file_name(path, &nextToken, '/');
	int realFileNameLen = strlen(realFileName);
	char* fileName = get_fat_file_name(realFileName);
	int fileEntryPos = 0;
	read_root_dir_again:;
	int i = 0;
	bool fileFound = false;
	read_sector(storage, rootDirCluster, 0, rootDirEntries*32/bytesPerSector, rootDir);
	while (rootDir[i] != 0) {
		fileEntryPos = i;
		if (rootDir[i+11] != 0x0F) {
			// Indicates this is not long file name entry
			if (strcmp((char*)((int)rootDir+i), fileName, 11)) {
				// File found
				// Check if long file name has the same name as the original name
				int lfnPos = i-32;
				int fnPos = 0;
				bool fileNameNotSame = false;
				if (rootDir[lfnPos+11] != 0x0F) {
					// This file has no Long File Name entry
					fileFound = true;
					break;
				}
				if (rootDir[lfnPos] > 0x40) {
					for (int j=0; j<5; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<6; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
					for (int j=0; j<2; j++) {
						if (fnPos >= realFileNameLen) {
							// File found
							fileFound = true;
							break;
						}
						if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
							fileNameNotSame = true;
							break;
						}
						fnPos++;
					}
					if (fileFound) {
						break;
					}
					if (fileNameNotSame) {
						break;
					}
				} else {
					while (rootDir[lfnPos] < 0x40) {
						int prevLfnPos = lfnPos;
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<5; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+1+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<6; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+14+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						for (int j=0; j<2; j++) {
							if (fnPos >= realFileNameLen) {
								// File found
								fileFound = true;
								break;
							}
							if (realFileName[fnPos] != rootDir[lfnPos+28+j*2]) {
								fileNameNotSame = true;
								break;
							}
							fnPos++;
						}
						if (fileFound) {
							break;
						}
						if (fileNameNotSame) {
							break;
						}
						lfnPos = prevLfnPos;
						lfnPos -= 32;
					}
				}
			}
		}
		i += 32;
	}
	if (!fileFound) {
		return FILE_DOESNT_EXIST;
	}
	if (fileFound) {
		i = fileEntryPos;
		int highCluster = (int)rootDir[i+21]*0x100+(int)rootDir[i+20];
		int lowCluster = (int)rootDir[i+27]*0x100+(int)rootDir[i+26];
		int clusterNumber = (highCluster<<16)|lowCluster;
		int attrib = rootDir[i+11];
		if ((attrib&0x10) != 0) {
			// This is a directory
			rootDirCluster = (clusterNumber-2)*sectorsPerCluster+dataStart;
			goto read_file_again;
		}
		int fileSize = ucharstouint(rootDir[i+31], rootDir[i+30], rootDir[i+29], rootDir[i+28]);
		int fileDataSector = (clusterNumber-2)*sectorsPerCluster+dataStart;
		int bufferSize = fileSize+((fileSize%bytesPerSector)?bytesPerSector:0);
		char* buffer = (char*)malloc(bufferSize);
		int originalBufferAddr = (int)buffer;
		read_again:;
		read_sector(storage, fileDataSector, 0, sectorsPerCluster, buffer);
		int fatPos = clusterNumber*3/2/bytesPerSector;
		char fat[bytesPerSector];
		read_sector(storage, startingSector+reservedSectors+fatPos, 0, 1, fat);
		int fatEntryPos = clusterNumber*3/2-(fatPos*bytesPerSector);
		int fatEntry = ucharstouint(fat[fatEntryPos+3], fat[fatEntryPos+2], fat[fatEntryPos+1], fat[fatEntryPos]);
		if (fatEntry >= 0x0FF8) {
			File* file = new File((char*)originalBufferAddr, fileSize);
			file->bytesPerSector = bytesPerSector;
			return file;
		}
		clusterNumber = fatEntry;
		fileDataSector = (clusterNumber-2)*sectorsPerCluster+dataStart;
		buffer = (char*)((int)buffer+bytesPerCluster);
		goto read_again;
	}
}
