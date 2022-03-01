#include <system.h>

static int memory_size;
static int kernel_area_end;
static int used_end = 0;

void init_memory(unsigned int end, unsigned int size) {
	kernel_area_end = end;
	used_end = end;
	memory_size = size;
}

static int count = 1;

char* malloc(int size) {
	alloc_t* info = (alloc_t*)kernel_area_end;
	if (info->magic != MEMORY_INFO_MAGIC) {
		// If the magic value is not 0xDEADBEEF, then the memory never allocated
		// So this is our first time allocating memory
		info->used = true;
		info->size = size+sizeof(alloc_t);
		info->magic = MEMORY_INFO_MAGIC;
		info->originalPtr = 0;
		used_end += sizeof(alloc_t)+size;
		return (char*)((uint32_t)info+sizeof(alloc_t));
	} else {
		while (true) {
			if (info->magic == MEMORY_INFO_MAGIC) {
				// If this info struct contains non-null magic value, then this area of memory is used before. So check them.
				if (!info->used) {
					if (info->size >= size+sizeof(alloc_t)) {
						info->used = true;
						if (info->size-size-sizeof(alloc_t) >= sizeof(alloc_t)+1) {
							info->size = size+sizeof(alloc_t);
						}
						return (char*)((uint32_t)info+sizeof(alloc_t));
					}
				}
			} else {
				// This area contains no magic value, so allocate this area.
				info->used = true;
				info->size = size+sizeof(alloc_t);
				info->magic = MEMORY_INFO_MAGIC;
				info->originalPtr = 0;
				used_end += sizeof(alloc_t)+size;
				return (char*)((uint32_t)info+sizeof(alloc_t));
			}
			info = (alloc_t*)((uint32_t)info+info->size);
		}
	}
}

void free(void* memory) {
	alloc_t* info = (alloc_t*)((uint32_t)memory-sizeof(alloc_t));
	if (info->originalPtr != 0) {
		memory = (void*)info->originalPtr;
		info = (alloc_t*)((uint32_t)memory-sizeof(alloc_t));
	}
	info->used = false;
	used_end -= sizeof(alloc_t)+info->size;
}

void* memcpy(void* dest, const void* source, int count) {
	char* src = (char*)source;
	char* dst = (char*)dest;
	for (int i=0; i<count; i++) {
		dst[i] = src[i];
	}
	return dest;
}

void* umemcpy(void* dest, void* source, size_t count) {
	unsigned char* src = (unsigned char*)source;
	char* dst = (char*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = uchartochar(src[i]);
	}
	return dest;
}

void* memcpy32(void* dest, void* source, size_t count) {
	int* src = (int*)source;
	int* dst = (int*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = src[i];
	}
	return dest;
}

char* memset(void* dest, int value, size_t count) {
	char* dst = (char*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

char* memset_u(void* dest, unsigned char value, size_t count) {
	unsigned char* dst = (unsigned char*)dest;
	for (int i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset16(void* dest, int value, size_t count) {
	short* dst = (short*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset16u(void* dest, unsigned short value, size_t count) {
	unsigned short* dst = (unsigned short*)dest;
	for (int i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset32(void* dest, int value, size_t count) {
	int* dst = (int*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset32u(void* dest, unsigned int value, size_t count) {
	unsigned int* dst = (unsigned int*)dest;
	for (int i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset64(void* dest, int value, size_t count) {
	long* dst = (long*)dest;
	for (size_t i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void* memset64u(void* dest, unsigned long value, size_t count) {
	unsigned long* dst = (unsigned long*)dest;
	for (int i=0; i<count; i++) {
		dst[i] = value;
	}
	return dest;
}

void *operator new(size_t size) {
    return malloc(size);
}

void *operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void *p, unsigned long l) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

int get_chunk_of_memory(int size) {
	int chunk_addr = used_end;
	used_end += size;
	return chunk_addr;
}

int get_used_memory() {
	return used_end;
}

char* malloc_align(int size, int boundary) {
	if (boundary < 16) {
		boundary = 16;
	}
	char* ptr = (char*)malloc(size+boundary-1);
	char* memory = (char*)(((uint32_t)ptr+boundary-1)&~(boundary-1));
	alloc_t* alloc = (alloc_t*)((uint32_t)memory-sizeof(alloc_t));
	alloc->size = size;
	alloc->used = true;
	alloc->originalPtr = (uint32_t)ptr;
	return memory;
}
