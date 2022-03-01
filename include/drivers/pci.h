#ifndef PCI_H
#define PCI_H

#include <system/datatype.h>

#define DEVICE_NOT_ATTACHED -1
#define DRV_INIT_OK 0

typedef struct {
	uint32_t bus;
	uint32_t slot;
	uint32_t func;
	uint32_t vendorId;
	uint32_t deviceId;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t classCode;
	uint32_t subclassCode;
	uint32_t progIF;
} Device;

typedef struct {
	char* name;
	int indexOfData;
	char* productName;
} DeviceIdCache;

extern int totalDevice;
extern int maxDevice;
extern Device** devices;
void init_devices();
void enumerate_devices();
uint16_t pci_read_short(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
uint32_t pci_read_int(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
void pci_write_int(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t value);
void pci_write_short(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint16_t value);
void pci_write_char(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint8_t value);
Device* get_device(uint32_t classCode, uint32_t subclassCode);
Device* get_device_2(uint32_t classCode, uint32_t subclassCode, uint32_t progIF);
Device* get_device_3(uint32_t vendorId, uint32_t deviceId);
char* get_device_name(Device* device);
void dump_devices();

#endif
