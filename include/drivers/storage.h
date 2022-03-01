#ifndef STORAGE_H
#define STORAGE_H

#define IDE 0
#define AHCI 1

#define FAT12 0
#define FAT16 1
#define FAT32 2
#define EXT2 3

#define HARDDISK 0
#define CDROM 1
#define FLOPPY 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FILE_ALREADY_EXISTS -1
#define UNKNOWN_FILE_SYSTEM -2
#define FILE_DOESNT_EXIST -3
#define FULL_PARTITION -4
#define CANNOT_UPDATE_DIRECTORY -5

typedef struct {
	int storageNumber;
	char label;
	int startingSector;
	int fs;
	int bytesPerSector;
	int totalSectors;
	int type;
	int partitionInfoLBA; //All the information about the partition (starting LBA, etc)
	int partitionInfoOffset; //Offset of the information inside content stored in partitionInfoLBA
} Partition;

typedef struct {
	int deviceType; //Commonly used for IDE
	int controllerType; //Commonly used for IDE
	int interface;
	void (*read_sector)(uint32_t* info);
	void (*write_sector)(uint32_t* info);
	Partition** partitions;
	int totalPartition;
	int maxPartitions;
	int type;
	int storageNumber;
} Storage;

class File {
public:
	char* path;
	char* content;
	int size;
	int id;
	int bytesPerSector; //only for specific uses such as user-mode file IO
	int readWritePos;
	
	File(char* content, int size) {
		this->content = content;
		this->size = size;
		readWritePos = 0;
	}
};

/* ONLY SPECIFIC TO USER-MODE APP, NOT KERNEL */
typedef struct {
    uint32_t st_dev;     /* ID of device containing file */
    uint32_t st_ino;     /* inode number */
    uint16_t st_mode;    /* protection */
    uint32_t st_nlink;   /* number of hard links */
    uint32_t st_uid;     /* user ID of owner */
    uint32_t st_gid;     /* group ID of owner */
    uint32_t st_rdev;    /* device ID (if special file) */
    long long st_size;    /* total size, in bytes */
    long st_blksize; /* blocksize for file system I/O */
    uint64_t st_blocks;  /* number of 512-Bytes blocks allocated */
    long long st_atime;   /* time of last access */
    long long st_mtime;   /* time of last modification */
    long long st_ctime;   /* time of last status change */
} storage_stat;

void add_storage(Storage* storage);
void add_partition(Storage* storage, Partition* partition);
Storage* getStorage(char label);
Partition* getPartition(char label);
char get_next_partition_label();
void write_sector(Storage* storage, int lba0, int lba1, int count, char* buffer);
void init_storages();
extern int totalPartition;
void set_storage_interface(int iface);
extern Storage** storages;
extern int totalStorage;
File* open_file(char* path);
char* get_file_name(char* path, int* nextToken, char separator);
char* get_fat_file_name(char* _fileName);
void read_sector(Storage* storage, int lba0, int lba1, int count, char* buffer);
void add_open_file(File* file);
File* get_open_file(int id);
void seek_file(int fileId, int ptr, int direction, int* seekEndPtr);
void read_file(int fileId, char* buffer, int len);
int write_file(char* path, char* content, int size);
void ide_read_sector_2(Storage* storage, int lba0, int lba1, int count, char* buffer);
int delete_file(char* path);
int make_directory(char* path);
int update_file(char* path, char* content, int size);
int format_partition(char* path, char* label, int bytesPerSector, int sectorsPerCluster);

#endif
