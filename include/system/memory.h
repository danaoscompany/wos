#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_INFO_MAGIC 0xDEADBEEF

#include <stddef.h>
#include <stdint.h>

typedef struct {
	bool used;
	uint32_t size;
	uint32_t magic;
	uint32_t originalPtr; //This is used only for aligned memory allocation
} __attribute__((packed)) alloc_t;

void init_memory(unsigned int end, unsigned int size);
char* malloc(int size);
char* memset(void* dest, int value, size_t count);
void* memcpy32(void* dest, void* source, size_t count);
void* umemcpy(void* dest, void* source, size_t count);
void* memset16(void* dest, int value, size_t count);
void* memset32(void* dest, int value, size_t count);
void* memset64(void* dest, int value, size_t count);
void free(void *mem);
void* memcpy(void* dest, const void* source, int count);
int get_used_memory();
char* malloc_align(int size, int boundary);
char* memset_u(void* dest, unsigned char value, size_t count);
void* memset16u(void* dest, unsigned short value, size_t count);
void* memset32u(void* dest, unsigned int value, size_t count);
void* memset64u(void* dest, unsigned long value, size_t count);

#endif
