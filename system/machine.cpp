#include <system.h>
#include <stdarg.h>

int cpuid_eax = 0;
int cpuid_edx = 0;
int msr_eax = 0;
int msr_ecx = 0;
int msr_edx = 0;

void cpuid(int code) {
	asm volatile("cpuid":"=a"(cpuid_eax),"=d"(cpuid_edx):"a"(code));
}

int read_cr(int number) {
	int eax = 0;
	if (number == 0) {
		asm volatile("mov %%cr0, %%eax":"=a"(eax));
	} else if (number == 1) {
		asm volatile("mov %%cr1, %%eax":"=a"(eax));
	} else if (number == 2) {
		asm volatile("mov %%cr2, %%eax":"=a"(eax));
	} else if (number == 3) {
		asm volatile("mov %%cr3, %%eax":"=a"(eax));
	} else if (number == 4) {
		asm volatile("mov %%cr4, %%eax":"=a"(eax));
	}
	return eax;
}

void write_cr(int number, int value) {
	if (number == 0) {
		asm volatile("mov %%eax, %%cr0"::"a"(value));
	} else if (number == 1) {
		asm volatile("mov %%eax, %%cr1"::"a"(value));
	} else if (number == 2) {
		asm volatile("mov %%eax, %%cr2"::"a"(value));
	} else if (number == 3) {
		asm volatile("mov %%eax, %%cr3"::"a"(value));
	} else if (number == 4) {
		asm volatile("mov %%eax, %%cr4"::"a"(value));
	}
}

void read_msr(int code) {
	asm volatile("rdmsr":"=a"(msr_eax),"=d"(msr_edx):"c"(code));
}

void write_msr(int code, int eax, int edx) {
	asm volatile("wrmsr"::"a"(eax),"d"(edx),"c"(code));
}

void wait() {
}

void stop() {
	while (1);
}

void loop() {
	while (1) {
		fill_bg(0x000000);
		flush();
	}
}

void ack() {
	ac97_play((uint8_t*)&ok_sound, 27726, 44100, 2);
}
