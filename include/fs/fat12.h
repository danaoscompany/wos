#ifndef FS_FAT12
#define FS_FAT12

File* fat12_open_file(char* path);
int fat12_write_file(char* path, char* content, int size);
int fat12_delete_file(char* path);
int fat12_update_file(char* path, char* content, int size);
int fat12_format(char* path, char* label, int bytesPerSector, int sectorsPerCluster);

#endif
