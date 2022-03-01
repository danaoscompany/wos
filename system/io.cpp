#include <system.h>

void outportb(int index, char value) {
	asm volatile("out %%al, %%dx"::"d"(index),"a"(value));
}

char inportb(int index) {
	char value;
	asm volatile("in %%dx, %%al":"=a"(value):"d"(index));
	return value;
}

void outportw(int index, short value) {
	asm volatile("out %%ax, %%dx"::"d"(index),"a"(value));
}

short inportw(int index) {
	short value;
	asm volatile("in %%dx, %%ax":"=a"(value):"d"(index));
	return value;
}

void outportd(int index, int value) {
	asm volatile("out %%eax, %%dx"::"d"(index),"a"(value));
}

void outportl(int index, int value) {
	asm volatile("out %%eax, %%dx"::"d"(index),"a"(value));
}

int inportd(int index) {
	int value;
	asm volatile("in %%dx, %%eax":"=a"(value):"d"(index));
	return value;
}

int inportl(int index) {
	int value;
	asm volatile("in %%dx, %%eax":"=a"(value):"d"(index));
	return value;
}

void outportb_u(int index, unsigned char value) {
	asm volatile("out %%al, %%dx"::"d"(index),"a"(value));
}

unsigned char inportb_u(int index) {
	unsigned char value;
	asm volatile("in %%dx, %%al":"=a"(value):"d"(index));
	return value;
}

void outportw_u(int index, unsigned short value) {
	asm volatile("out %%ax, %%dx"::"d"(index),"a"(value));
}

unsigned short inportw_u(int index) {
	unsigned short value;
	asm volatile("in %%dx, %%ax":"=a"(value):"d"(index));
	return value;
}

void outportd_u(int index, unsigned int value) {
	asm volatile("out %%eax, %%dx"::"d"(index),"a"(value));
}

unsigned int inportd_u(int index) {
	unsigned int value;
	asm volatile("in %%dx, %%eax":"=a"(value):"d"(index));
	return value;
}

void mmio_write_8(uint32_t addr, uint8_t value) {
	uint8_t* address = (uint8_t*)addr;
	address[0] = value;
}

uint8_t mmio_read_8(uint32_t addr) {
	uint8_t* address = (uint8_t*)addr;
	return address[0];
}

void mmio_write_16(uint32_t addr, uint16_t value) {
	uint16_t* address = (uint16_t*)addr;
	address[0] = value;
}

uint16_t mmio_read_16(uint32_t addr) {
	uint16_t* address = (uint16_t*)addr;
	return address[0];
}

void mmio_write_32(uint32_t addr, uint32_t value) {
	uint32_t* address = (uint32_t*)addr;
	address[0] = value;
}

uint32_t mmio_read_32(uint32_t addr) {
	uint32_t* address = (uint32_t*)addr;
	return address[0];
}

void mmio_write_64(uint32_t addr, uint64_t value) {
	uint64_t* address = (uint64_t*)addr;
	address[0] = value;
}

uint64_t mmio_read_64(uint32_t addr) {
	uint64_t* address = (uint64_t*)addr;
	return address[0];
}
