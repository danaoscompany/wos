#include <system.h>

Device* ac97Device = NULL;
bool playing = true;
static uint8_t currEntry = 0;
static uint32_t audioPtr = 0;
static uint32_t samplesConsumed = 0;
static uint8_t currentChannelCount = 0;
static uint32_t currentTotalSamples = 0;
static uint8_t* currentAudioBuffer = 0;
static BufferDescriptorList bdl[32];

void init_ac97() {
	for (int i = 0; i < totalDevice; i++) {
		Device* device = devices[i];
		if (device != NULL) {
			if (device->classCode == 0x4 && device->subclassCode == 0x1) {
				ac97Device = device;
				uint32_t command = pci_read_int(device->bus, device->slot, device->func, 0x4);
				device->bar0 = device->bar0 & 0xFFFE;
				device->bar1 = device->bar1 & 0xFFFE;
				command |= 0x1;
				command |= 0x4;
				pci_write_int(device->bus, device->slot, device->func, 0x4, command);
				break;
			}
		}
	}
}

void ac97_handler(Register* r) {
	if (ac97Device != NULL) {
		outportw(ac97Device->bar1+0x16, 0x1C);
		if (samplesConsumed >= currentTotalSamples) {
			return;
		}
		currEntry = inportb(ac97Device->bar1 + 0x1A);
		bdl[currEntry].phys = (uint32_t)((uint32_t)currentAudioBuffer + audioPtr);
		bdl[currEntry].samples = 32 * 1024;
		if (samplesConsumed == currentTotalSamples - 1) {
			bdl[currEntry].misc = 0xC000;
		}
		else {
			bdl[currEntry].misc = 0x8000;
		}
		audioPtr += (32 * 1024 * currentChannelCount);
		outportb(ac97Device->bar1 + 0x15, currEntry);
		samplesConsumed++;
	}
}

void ac97_play(uint8_t* buffer, uint32_t length, uint16_t sampleRate, uint8_t channelCount) {
	if (ac97Device == NULL) return;
	currentAudioBuffer = buffer;
	currentChannelCount = channelCount;
	Device* device = ac97Device;
	outportw(ac97Device->bar1 + 0x16, 0x1C);
	// Reset
	outportd(device->bar1 + 0x1b, 0x2);
	sleep(1);
	// Write sample rate
	outportw(device->bar0 + 0x2c, sampleRate);
	sleep(1);
	// Unmute
	uint16_t unmuteValue = inportw(device->bar1 + 0x4c);
	unmuteValue |= 0x1;
	outportw(device->bar1 + 0x4c, unmuteValue);
	// Set maximum volume
	outportw(device->bar0 + 0x2, 0);
	sleep(1);
	outportw(device->bar0 + 0x18, 0);
	sleep(1);
	outportd(device->bar1 + 0x10, (uint32_t)&bdl);
	currEntry = 0;
	audioPtr = 0;
	samplesConsumed = 0;
	uint32_t totalSamples = length / (32 * 1024 * channelCount);
	currentTotalSamples = totalSamples;
	if ((length % (32 * 1024 * channelCount)) != 0) {
		totalSamples++;
	}
	bdl[currEntry].phys = (uint32_t)((uint32_t)buffer + audioPtr);
	bdl[currEntry].samples = 32 * 1024;
	if (samplesConsumed == totalSamples-1) {
		bdl[currEntry].misc = 0xC000;
	} else {
		bdl[currEntry].misc = 0x8000;
	}
	audioPtr += (32 * 1024 * channelCount);
	samplesConsumed++;
	outportb(device->bar1 + 0x15, currEntry);
	irq_set_handler(11, ac97_handler);
	outportb(device->bar1 + 0x1b, 0x1D);
}

void ac97_pause() {
	if (ac97Device != NULL) {
		uint8_t value = inportb(ac97Device->bar1 + 0x1B);
		value &= ~1;
		outportb(ac97Device->bar1 + 0x1B, value);
	}
}

void ac97_resume() {
	if (ac97Device != NULL) {
		uint8_t value = inportb(ac97Device->bar1 + 0x1B);
		value |= 1;
		outportb(ac97Device->bar1 + 0x1B, value);
	}
}

void testAC97() {
	for (int i = 0; i < totalDevice; i++) {
		Device* device = devices[i];
		if (device != NULL) {
			if (device->classCode == 0x4 && device->subclassCode == 0x1) {
				uint32_t command = pci_read_int(device->bus, device->slot, device->func, 0x4);
				device->bar0 = device->bar0 & 0xFFFE;
				device->bar1 = device->bar1 & 0xFFFE;
				printf("Device BAR0: %x\n", device->bar0);
				printf("Device BAR1: %x\n", device->bar1);
				command |= 0x1;
				command |= 0x4;
				pci_write_int(device->bus, device->slot, device->func, 0x4, command);
				// Write sample rate
				outportw(device->bar0 + 0x2c, 44100);
				sleep(1);
				// Unmute
				uint16_t unmuteValue = inportw(device->bar1 + 0x4c);
				unmuteValue |= 0x1;
				outportw(device->bar1 + 0x4c, unmuteValue);
				// Set maximum volume
				outportw(device->bar0 + 0x2, 0);
				sleep(1);
				outportw(device->bar0 + 0x18, 0);
				sleep(1);
				BufferDescriptorList bdl[32];
				outportd(device->bar1 + 0x10, (uint32_t)&bdl);
				uint8_t currEntry = 0;
				uint32_t audioPtr = 0;
				bdl[currEntry].phys = (uint32_t)((uint32_t)&song + audioPtr);
				bdl[currEntry].samples = 32 * 1024;
				bdl[currEntry].misc = 0x0000;
				audioPtr += (32 * 1024 * 2);
				outportb(device->bar1 + 0x15, currEntry);
				outportb(device->bar1 + 0x1b, 1);
				while (true) {
					while (true) {
						uint8_t currProcEntry = inportb(device->bar1 + 0x14);
						if (currProcEntry == currEntry) {
							break;
						}
					}
					currEntry = inportb(device->bar1 + 0x1A);
					bdl[currEntry].phys = (uint32_t)((uint32_t)&song + audioPtr);
					bdl[currEntry].samples = 32 * 1024;
					bdl[currEntry].misc = 0x0000;
					audioPtr += (32 * 1024 * 2);
					outportb(device->bar1 + 0x15, currEntry);
				}
			}
		}
	}
}