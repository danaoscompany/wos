#ifndef FS_FAT16
#define FS_FAT16

File* fat16_open_file(char* path);
int fat16_write_file(char* path, char* content, int size);
int fat16_delete_file(char* path);
int fat16_update_file(char* path, char* content, int size);
int fat16_format(char* path, char* label, int bytesPerSector, int sectorsPerCluster);

#endif
