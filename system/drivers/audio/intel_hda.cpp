#include <system.h>

static Device* device;
static uint32_t bar;
static uint32_t corbAddr;

void init_intel_hda() {
	device = get_audio_device();
	bar = device->bar0;
	uint32_t value = mmio_read_32(bar+0x08);
	value |= 0x01;
	mmio_write_32(bar+0x08, value);
	while ((mmio_read_32(bar+0x08)&0x01)==0);
	sleep(1);
	// Stop CORB
	uint8_t value8 = mmio_read_8(bar+0x4C);
	if (((value8>>1)&1)!=0) {
		// CORB is running, so stop it
		value8 &= ~(1<<1);
		mmio_write_8(bar+0x4C, value8);
		while (((mmio_read_8(bar+0x4C)>>1)&1)!=0);
	}
	corbAddr = (uint32_t)malloc_align(1024, 128);
	mmio_write_32(bar+0x44, 0x00);
	mmio_write_32(bar+0x40, corbAddr);
	value8 = mmio_read_8(bar+0x4E);
	value8 |= 0x02;
	mmio_write_8(bar+0x4E, value8);
	// Reset CORB RP
	value = (uint32_t)mmio_read_16(bar+0x4A);
	value |= (1<<15);
	mmio_write_16(bar+0x4A, value);
	sleep(500);
	//while ((mmio_read_16(bar+0x4A)>>15&1)==0);
	value = (uint32_t)mmio_read_16(bar+0x4A);
	value &= ~(1<<15);
	mmio_write_16(bar+0x4A, value);
	//while ((mmio_read_16(bar+0x4A)>>15&1)!=0);
	sleep(500);
	mmio_write_16(bar+0x48, 0);
	value8 = mmio_read_8(bar+0x4C);
	value8 |= (1<<1);
	mmio_write_8(bar+0x4C, value8);
	while (((mmio_read_8(bar+0x4C)>>1)&1)==0);
	printf("Done");
}
