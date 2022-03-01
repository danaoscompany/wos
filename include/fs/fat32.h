#ifndef FS_FAT32
#define FS_FAT32

File* fat32_open_file(char* path);
int fat32_write_file(char* path, char* content, int size);
int fat32_delete_file(char* path);
int fat32_update_file(char* path, char* content, int size);
int fat32_format(char* path, char* label, int bytesPerSector, int sectorsPerCluster);

#endif
