#ifndef IDE_H
#define IDE_H

#include "storage.h"

#define MASTER_DEVICE 0
#define SLAVE_DEVICE 1
#define PRIMARY_CONTROLLER 0
#define SECONDARY_CONTROLLER 0

void init_ide();
Device* get_ide_device();
uint32_t ide_get_primary_bar();
uint32_t ide_get_secondary_bar();
void ide_read_sector(Storage* storage, int lba0, int lba1, int count, char* buffer);
void ide_write_sector(Storage* storage, int lba0, int lba1, int count, char* buffer);

#endif
