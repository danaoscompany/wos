#include <libsystem.h>

void call(int eax, int ebx, int ecx, int edx) {
	asm volatile("int 48"::"a"(eax),"b"(ebx),"c"(ecx),"d"(edx));
}
