[BITS 16]
ORG 0x7C00
start:
	jmp 0x00:boot
	
	times 8-($-$$) db 0
	
	bi_PrimaryVolumeDescriptor resd 1
	bi_BootFileLoc resd 1
	bi_BootFileLength resd 1
	bi_Checksum resd 1
	bi_Reserved resb 40

boot:
	mov [boot_device], dl
	
	mov ax, 0
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xFFFF
	
	; Loading loader from CD
	mov ax, 16
	mov bx, 1
	mov ecx, 0x500
	call read_sector
	
	; Find root directory
	mov si, 0x500
	add si, 156
	mov eax, [si+10]
	mov [root_dir_size], eax
	mov ebx, 2048
	xor edx, edx
	div ebx
	cmp edx, 0
	je .1
	add eax, 1
	.1:
	mov bx, ax
	mov ax, [si+2]
	mov ecx, 0x8C00
	call read_sector
	
	; Search for kernel
	mov esi, 0x8C00
	xor ecx, ecx
	.loop1:
	cmp ecx, [root_dir_size]
	jge .not_found
	push esi
	push ecx
	add esi, 33
	mov edi, loader_filename
	mov ecx, 12
	rep cmpsb
	je .found
	pop ecx
	pop esi
	mov al, [esi]
	movzx eax, al
	add esi, eax
	add ecx, eax
	jmp .loop1
	
	.not_found:
	mov si, text1
	call print_str
	jmp $
	
	.found:
	pop ecx
	pop esi
	
	mov eax, [esi+10]
	mov ebx, 2048
	xor edx, edx
	div ebx
	cmp edx, 0
	je .2
	add eax, 1
	.2:
	mov bx, ax
	mov ax, [esi+2]
	mov ecx, 0x8C00
	call read_sector
	
	in al, 0x92
	or al, 2
	out 0x92, al
	cli
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	lgdt [gdt_ptr]
	jmp 0x8:ProtectedMode
	
[BITS 32]
	ProtectedMode:
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov sp, 0xFFFF
	
	mov esi, 0x8C00
	mov edi, ELFHeader
	mov ecx, 52
	rep movsb
	
	mov cx, [ELFHeader.shnum]
	movzx ecx, cx
	
	.loop1:
	push ecx
	
	mov esi, 0x8C00
	add esi, [ELFHeader.shoff]
	mov edi, ELFSectionHeader
	mov ecx, 40
	rep movsb
	
	cmp dword [ELFSectionHeader.type], 8
	je .zero
	
	mov esi, 0x8C00
	add esi, [ELFSectionHeader.offs]
	mov edi, [ELFSectionHeader.addr]
	mov ecx, [ELFSectionHeader.size]
	rep movsb
	jmp .skip
	
	.zero:
	mov edi, [ELFSectionHeader.addr]
	mov al, 0
	mov ecx, [ELFSectionHeader.size]
	;rep stosb
	
	.skip:
	add dword [ELFHeader.shoff], 40
	pop ecx
	loop .loop1
	
	jmp dword [ELFHeader.entry]
	
	jmp $

ELFHeader:
	.ident times 16 db 0
	.type dw 0
	.machine dw 0
	.version dd 0
	.entry dd 0
	.phoff dd 0
	.shoff dd 0
	.flags dd 0
	.ehsize dw 0
	.phentsize dw 0
	.phnum dw 0
	.shentsize dw 0
	.shnum dw 0
	.shstrndx dw 0

ELFSectionHeader:
	.name dd 0
	.type dd 0
	.flags dd 0
	.addr dd 0
	.offs dd 0
	.size dd 0
	.link dd 0
	.info dd 0
	.addralign dd 0
	.entsize dd 0

ELFProgramHeader:
	.type dd 0
	.offset dd 0
	.vaddr dd 0
	.paddr dd 0
	.filesz dd 0
	.memsz dd 0
	.flags dd 0
	.align dd 0

dump32:
	; EAX = buffer
	; EBX = count
	pusha
	mov esi, eax
	mov ecx, ebx
	mov edi, 0xB8000
	mov eax, 0;[text_y]
	mov ebx, 80
	mul ebx
	mov ebx, 2
	mul ebx
	add edi, eax
	.loop:
	lodsb
	stosb
	mov al, 0x0F
	stosb
	loop .loop
	popa
	pusha
	mov eax, ebx
	mov ebx, 80
	xor edx, edx
	div ebx
	cmp edx, 0
	jne .1
	jmp .2
	.1:
	add eax, 1
	.2:
	mov [text_y], eax
	popa
	ret

[BITS 16]
	
	text1 db "start file not found", 0
	text2 db "start file found", 0
	loader_filename db "LOADER.BIN;1", 0
	root_dir_size dd 0

gdt_ptr:
	dw 3*8-1
	dd gdt

gdt:
	; Null GDT
	dq 0
	; Code segment
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 0x9A
	db 0xCF
	db 0x0
	; Data segment
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 0x92
	db 0xCF
	db 0x0

strlen:
	; AX = string
	pusha
	mov si, ax
	xor cx, cx
	.loop:
	lodsb
	cmp al, 0
	je .done
	add cx, 1
	jmp .loop
	.done:
	mov [.len], cx
	popa
	mov cx, [.len]
	ret
	
	.len dw 0
	
dump:
	; AX = buffer
	; BX = count
	pusha
	mov si, ax
	mov cx, bx
	mov bx, 0xB800
	mov es, bx
	mov di, 0
	mov ax, 0;[text_y]
	mov bx, 80
	mul bx
	mov bx, 2
	mul bx
	add di, ax
	.loop:
	lodsb
	mov byte [es:di], al
	mov byte [es:di+1], 0x0F
	add di, 2
	loop .loop
	popa
	pusha
	mov ax, bx
	mov bx, 80
	xor dx, dx
	div bx
	cmp dx, 0
	jne .1
	jmp .2
	.1:
	add ax, 1
	.2:
	mov [text_y], ax
	popa
	ret

print_str:
	; SI = text
	pusha
	mov bx, 0xB800
	mov es, bx
	mov di, 0
	mov ax, [text_y]
	mov bx, 80
	mul bx
	mov bx, 2
	mul bx
	add di, ax
	.loop:
	lodsb
	cmp al, 0
	je .done
	mov byte [es:di], al
	mov byte [es:di+1], 0x0F
	add di, 2
	jmp .loop
	.done:
	popa
	add word [text_y], 1
	ret

numlen:
	; AX = number
	pusha
	cmp ax, 0
	je .0
	xor cx, cx
	.loop:
	cmp ax, 0
	jle .done
	mov bx, 10
	xor dx, dx
	div bx
	add cx, 1
	jmp .loop
	.done:
	mov [.count], cx
	popa
	mov cx, [.count]
	ret
	
	.0:
	popa
	mov ax, 1
	ret
	
	.count dw 0

print_num:
	; AX = number
	pusha
	push ax
	call numlen
	sub ax, 1
	mov bx, 2
	mul bx
	mov cx, ax
	mov bx, 0xB800
	mov es, bx
	mov di, 0
	mov ax, [text_y]
	mov bx, 80
	mul bx
	mov bx, 2
	mul bx
	add di, ax
	add di, cx
	pop ax
	cmp ax, 0
	je .0
	.loop:
	cmp ax, 0
	je .done
	mov bx, 10
	xor dx, dx
	div bx
	add dl, '0'
	mov byte [es:di], dl
	mov byte [es:di+1], 0x0F
	sub di, 2
	jmp .loop
	.done:
	popa
	add word [text_y], 1
	ret
	
	.0:
	mov byte [es:di], '0'
	mov byte [es:di+1], 0x0F
	popa
	add word [text_y], 1
	ret

read_sector:
	; AX = LBA to read
	; BX = count
	; ECX = buffer
	pusha
	mov [read_sector_packet.num_blocks], bx
	mov [read_sector_packet.buffer], ecx
	mov [read_sector_packet.lba], ax
	mov ah, 42h
	mov dl, [boot_device]
	mov si, read_sector_packet
	int 13h
	jc .error
	popa
	ret
	
	.error:
	mov si, .text_err
	call print_str
	popa
	ret
	
	.text_err db "Error reading sector:", 0
	
read_sector_packet:
	.size db 8
	.rsv0 db 0
	.num_blocks dw 0
	.buffer dd 0
	.lba dq 0

text_y dw 0
boot_device db 0

times 4094-($-$$) db 0
dw 0xAA55
