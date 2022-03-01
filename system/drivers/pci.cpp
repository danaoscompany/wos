#include <system.h>

int totalDevice = 0;
Device** devices;
int maxDevice = 64;
char* deviceIds;
DeviceIdCache** deviceIdCaches;
int maxDeviceIdCaches = 10;
int totalDeviceIdCache = 0;

void init_devices() {
	devices = (Device**)((uint32_t)malloc(sizeof(Device*)*maxDevice));
	deviceIds = (char*)&device_ids;
}

void add_device(Device* dev) {
	if (totalDevice >= maxDevice) {
		maxDevice *= 2;
		Device** tmp = (Device**)malloc(sizeof(Device*)*maxDevice);
		for (int i=0; i<totalDevice; i++) {
			tmp[i] = devices[i];
		}
		free(devices);
		devices = tmp;
	}
	devices[totalDevice] = dev;
	totalDevice++;
}

void add_device_id_cache(DeviceIdCache* devCache) {
	if (totalDeviceIdCache >= maxDeviceIdCaches) {
		maxDeviceIdCaches *= 2;
		DeviceIdCache** tmp = (DeviceIdCache**)malloc(sizeof(DeviceIdCache*)*maxDeviceIdCaches);
		for (int i=0; i<totalDeviceIdCache; i++) {
			tmp[i] = deviceIdCaches[i];
		}
		free(deviceIdCaches);
		deviceIdCaches = tmp;
	}
	deviceIdCaches[totalDeviceIdCache] = devCache;
	totalDeviceIdCache++;
}

void enumerate_devices() {
	for (uint32_t bus=0; bus<255; bus++) {
		for (uint32_t slot=0; slot<31; slot++) {
			for (uint32_t func=0; func<7; func++) {
				uint32_t vendorId = (uint32_t)pci_read_int(bus, slot, func, 0)&0xFFFF;
				if (vendorId == 0xFFFF) {
					continue;
				}
				uint32_t deviceId = (uint32_t)pci_read_int(bus, slot, func, 0)>>16;
				uint32_t bar0 = pci_read_int(bus, slot, func, 16);
				uint32_t bar1 = pci_read_int(bus, slot, func, 20);
				uint32_t bar2 = pci_read_int(bus, slot, func, 24);
				uint32_t bar3 = pci_read_int(bus, slot, func, 28);
				uint32_t bar4 = pci_read_int(bus, slot, func, 32);
				uint32_t bar5 = pci_read_int(bus, slot, func, 36);
				uint32_t progIF = (uint32_t)pci_read_short(bus, slot, func, 8);
				progIF = (progIF>>8)&0xFF;
				uint32_t cls = (uint32_t)pci_read_short(bus, slot, func, 10);
				uint32_t classCode = cls>>8&0xFF;
				uint32_t subclassCode = cls&0xFF;
				Device* dev = (Device*)malloc(sizeof(Device));
				dev->bus = bus;
				dev->slot = slot;
				dev->func = func;
				dev->vendorId = vendorId;
				dev->deviceId = deviceId;
				dev->bar0 = bar0;
				dev->bar1 = bar1;
				dev->bar2 = bar2;
				dev->bar3 = bar3;
				dev->bar4 = bar4;
				dev->bar5 = bar5;
				dev->classCode = classCode;
				dev->subclassCode = subclassCode;
				dev->progIF = progIF;
				add_device(dev);
			}
		}
	}
}

uint16_t pci_read_short(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset) {
    uint32_t address = (bus<<16)|(slot<<11)|(func<<8)|(offset&0xfc)|0x80000000;
    outportl(0xCF8, address);
    return (uint16_t)((inportl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint32_t pci_read_int(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset) {
    uint32_t address = (bus<<16)|(slot<<11)|(func<<8)|(offset&0xfc)|0x80000000;
    outportl(0xCF8, address);
    return (uint32_t)inportl(0xCFC);
}

void pci_write_int(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t value) {
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outportl(0xCF8, address);
    outportl(0xCFC, value);
}

void pci_write_short(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint16_t value) {
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
	outportl(0xCF8, address);
	outportw(0xCFC, value);
}

void pci_write_char(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint8_t value) {
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
	outportl(0xCF8, address);
	outportb(0xCFC, value);
}

Device* get_device(uint32_t classCode, uint32_t subclassCode) {
	for (uint32_t i=0; i<totalDevice; i++) {
		if (devices[i]->classCode == classCode && devices[i]->subclassCode == subclassCode) {
			return devices[i];
		}
	}
	return NULL;
}

Device* get_device_2(uint32_t classCode, uint32_t subclassCode, uint32_t progIF) {
	for (uint32_t i=0; i<totalDevice; i++) {
		if (devices[i]->classCode == classCode && devices[i]->subclassCode == subclassCode && devices[i]->progIF == progIF) {
			return devices[i];
		}
	}
	return NULL;
}

Device* get_device_3(uint32_t vendorId, uint32_t deviceId) {
	for (uint32_t i=0; i<totalDevice; i++) {
		if (devices[i]->vendorId == vendorId && devices[i]->deviceId == deviceId) {
			return devices[i];
		}
	}
	return NULL;
}

char* get_device_name(Device* device) {
	/* Check if device id caches contains non-zero device id cache.
	If yes, check if the device id of the device is available at the device id caches.
	If it is found, then use the information to detect the name of the device. */
	char* deviceName = concat("(Device)", inttohexstr(device->deviceId));
	int deviceNameLen = strlen(deviceName);
	for (int i=0; i<totalDeviceIdCache; i++) {
		DeviceIdCache* cache = deviceIdCaches[i];
		if (strcmp(cache->name, deviceName, strlen(deviceName))) {
			return cache->productName;
		}
	}
	char* name = concat("(Vendor)", inttohexstr(device->vendorId));
	int index = search_string(deviceIds, name, 0, 0);
	int nextVendorStringIndex = get_index_of_string(deviceIds, "(Vendor)", index+1, 0);
	while (index < nextVendorStringIndex) {
		if (strcmpf(deviceIds, deviceName, deviceNameLen, index, 0)) {
			int endIndex = get_char_index_from_string(deviceIds, '\n', index, 0);
			deviceIds[endIndex] = 0;
			int productNameLen = endIndex-index-deviceNameLen-2;
			char* productName = (char*)malloc(productNameLen+1);
			memcpy(productName, (char*)((int)deviceIds+index+deviceNameLen+2), productNameLen);
			productName[productNameLen] = 0;
			return productName;
		}
		index++;
	}
	return NULL;
}

void dump_devices() {
	for (int i=0; i<totalDevice; i++) {
		printf("%x\n", devices[i]->deviceId);
	}
}
