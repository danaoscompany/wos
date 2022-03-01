#include <system.h>

static Device* device = NULL;
static uint32_t bar;

void ide_harddisk_read_sector(uint32_t* info) {
	if (device == NULL) {
		device = get_ide_device();
	}
	uint32_t lba0 = info[0];
	uint32_t lba1 = info[1];
	uint32_t count = info[2];
	char* buffer = (char*)info[3];
	int deviceType = info[4];
	int controllerType = info[5];
	if (controllerType == 0) {
		bar = ide_get_primary_bar();
	} else if (controllerType == 1) {
		bar = ide_get_secondary_bar();
	}
	outportb(bar+1, 0);
	outportb(bar+2, count&0xFF);
	outportb(bar+3, lba0&0xFF);
	outportb(bar+4, lba0>>8&0xFF);
	outportb(bar+5, lba0>>16&0xFF);
	outportb(bar+6, (deviceType<<4)|(1<<6)|(lba0>>24&0xF));
	outportb(bar+7, 0x20);
	while ((inportb(bar+7)&0x80)!=0);
	int k = 0;
	for (int i=0; i<count; i++) {
		for (int j=0; j<256; j++) {
			short data = inportw(bar);
			buffer[k] = data&0xFF;
			buffer[k+1] = data>>8&0xFF;
			k += 2;
		}
	}
	while ((inportb(bar+7)&0x80)!=0);
	free(info);
}

void ide_harddisk_write_sector(uint32_t* info) {
	if (device == NULL) {
		device = get_ide_device();
	}
	uint32_t lba0 = info[0];
	uint32_t lba1 = info[1];
	uint32_t count = info[2];
	char* buffer = (char*)info[3];
	int deviceType = info[4];
	int controllerType = info[5];
	if (controllerType == 0) {
		bar = ide_get_primary_bar();
	} else if (controllerType == 1) {
		bar = ide_get_secondary_bar();
	}
	outportb(bar+1, 0);
	outportb(bar+2, count&0xFF);
	outportb(bar+3, lba0&0xFF);
	outportb(bar+4, lba0>>8&0xFF);
	outportb(bar+5, lba0>>16&0xFF);
	outportb(bar+6, (deviceType<<4)|(1<<6)|(lba0>>24&0xF));
	outportb(bar+7, 0x30);
	while ((inportb(bar+7)&0x80)!=0);
	int k = 0;
	for (int i=0; i<count; i++) {
		for (int j=0; j<256; j++) {
			short data = (short)((int)buffer[k]|((int)buffer[k]<<8));
			outportw(bar, data);
			k += 2;
		}
	}
	while ((inportb(bar+7)&0x80)!=0);
	free(info);
}
