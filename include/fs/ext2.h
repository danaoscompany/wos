#ifndef EXT2_H
#define EXT2_H

#define UNSUPPORTED_BLOCK_SIZE -1

File* ext2_open_file(char* path);
int ext2_write_file(char* path, char* content, int size);
int ext2_delete_file(char* path);
int ext2_update_file(char* path, char* content, int size);
int ext2_format(char* path, char* label, int bytesPerSector, int sectorsPerCluster);

#endif