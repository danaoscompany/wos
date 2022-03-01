[BITS 32]

global flush_sse
flush_sse:
	; 1 = screen (EAX)
	; 2 = current screen (EBX)
	; 3 = copy size (ECX)
	push ebp
	mov ebp, esp
	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]
	mov esp, ebp
	pop ebp
	.loop:
	cmp ecx, 0
	jle .done
		movups xmm0, [esi]
		movups [edi], xmm0
		add edi, 16
		add esi, 16
	sub ecx, 16
	jmp .loop
	.done:
	ret
