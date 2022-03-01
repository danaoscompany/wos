[BITS 32]

global start
start:
	extern main
	call main
	ret
