#ifndef IO_H
#define IO_H

void outportb(int index, char value);
char inportb(int index);
void outportw(int index, short value);
short inportw(int index);
void outportd(int index, int value);
void outportl(int index, int value);
int inportd(int index);
int inportl(int index);
void outportb_u(int index, unsigned char value);
unsigned char inportb_u(int index);
void outportw_u(int index, unsigned short value);
unsigned short inportw_u(int index);
void outportd_u(int index, unsigned int value);
unsigned int inportd_u(int index);
void mmio_write_8(uint32_t addr, uint8_t value);
uint8_t mmio_read_8(uint32_t addr);
void mmio_write_16(uint32_t addr, uint16_t value);
uint16_t mmio_read_16(uint32_t addr);
void mmio_write_32(uint32_t addr, uint32_t value);
uint32_t mmio_read_32(uint32_t addr);
void mmio_write_64(uint32_t addr, uint64_t value);
uint64_t mmio_read_64(uint32_t addr);

#endif
