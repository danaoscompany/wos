[BITS 32]

global _start
_start:
	mov eax, 0x11
	mov ebx, 0
	mov ecx, 0
	mov edx, 0
	int 48
	jmp $
	extern main
	call main
	ret
