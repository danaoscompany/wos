#ifndef IDE_HARDDISK_H
#define IDE_HARDDISK_H

void ide_harddisk_read_sector(uint32_t* info);
void ide_harddisk_write_sector(uint32_t* info);

#endif
