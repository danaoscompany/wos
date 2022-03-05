void sys_call(int eax, int ebx, int ecx, int edx) {
	asm volatile("int $48"::"a"(eax),"b"(ebx),"c"(ecx),"d"(edx));
}

void sys_call2(int eax, int ebx, int ecx, int edx) {
	asm volatile("int $48"::"a"(eax),"b"(ebx),"c"(ecx),"d"(edx));
}

void test_func_from_libtest() {
	sys_call(0x11, 0, 0, 0);
}
