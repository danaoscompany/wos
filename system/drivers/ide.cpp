#include <system.h>

static Device* device;
static uint32_t pBar;
static uint32_t sBar;

void init_ide() {
	device = get_device(0x1, 0x1);
	if (device == NULL) {
		return;
	}
	pBar = device->bar0&0xFFFFFFFC;
	if (pBar == 0 || pBar == 1) {
		pBar = 0x1F0;
	}
	sBar = device->bar2&0xFFFFFFFC;
	if (sBar == 0 || sBar == 1) {
		sBar = 0x170;
	}
	// Send IDENTIFY_DEVICE command
	outportb(pBar+1, 0);
	outportb(pBar+2, 0);
	outportb(pBar+3, 0);
	outportb(pBar+4, 0);
	outportb(pBar+5, 0);
	outportb(pBar+6, MASTER_DEVICE<<4);
	outportb(pBar+7, 0xEC);
	while ((inportb(pBar+7)&0x80)!=0);
	int type;
	if ((inportb(pBar+7)&0x1)==0) {
		type = HARDDISK;
	} else {
		type = CDROM;
	}
	Storage* storage = (Storage*)malloc(sizeof(Storage));
	storage->deviceType = MASTER_DEVICE;
	storage->controllerType = PRIMARY_CONTROLLER;
	storage->interface = IDE;
	storage->type = type;
	add_storage(storage);
	outportb(sBar+1, 0);
	outportb(sBar+2, 0);
	outportb(sBar+3, 0);
	outportb(sBar+4, 0);
	outportb(sBar+5, 0);
	outportb(sBar+6, MASTER_DEVICE<<4);
	outportb(sBar+7, 0xEC);
	while ((inportb(pBar+7)&0x80)!=0);
	if ((inportb(sBar+7)&0x1)==0) {
		type = HARDDISK;
	} else {
		type = CDROM;
	}
	storage = (Storage*)malloc(sizeof(Storage));
	//storage->label = get_next_partition_label();
	storage->deviceType = MASTER_DEVICE;
	storage->controllerType = SECONDARY_CONTROLLER;
	storage->interface = IDE;
	storage->type = type;
	add_storage(storage);
}

void ide_read_sector(Storage* storage, int lba0, int lba1, int count, char* buffer) {
	uint32_t bar;
	int deviceType = storage->deviceType;
	int controllerType = storage->controllerType;
	if (controllerType == 0) {
		bar = ide_get_primary_bar();
	} else if (controllerType == 1) {
		bar = ide_get_secondary_bar();
	}
	while ((inportb(bar+7)&0x80)!=0);
	if (storage->type == HARDDISK) {
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
			while ((inportb(bar+7)&0x80)!=0);
		}
		while ((inportb(bar+7)&0x80)!=0);
	}
}

void ide_write_sector(Storage* storage, int lba0, int lba1, int count, char* buffer) {
	uint32_t bar;
	int deviceType = storage->deviceType;
	int controllerType = storage->controllerType;
	if (controllerType == 0) {
		bar = ide_get_primary_bar();
	} else if (controllerType == 1) {
		bar = ide_get_secondary_bar();
	}
	while ((inportb(bar+7)&0x80)!=0);
	if (storage->type == HARDDISK) {
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
				short data = (short)(((int)buffer[k])|(int)buffer[k+1]<<8);
				outportw(bar, data);
				k += 2;
			}
			while ((inportb(bar+7)&0x80)!=0);
		}
		while ((inportb(bar+7)&0x80)!=0);
	}
}

Device* get_ide_device() {
	return device;
}

uint32_t ide_get_primary_bar() {
	return pBar;
}

uint32_t ide_get_secondary_bar() {
	return sBar;
}
