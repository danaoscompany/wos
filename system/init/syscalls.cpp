#include <system.h>
#include <stdarg.h>

extern "C" void syscall();

void init_syscalls() {
	idt_set_gate(48, (unsigned)syscall, 0x08, 0x8E);
}

void syscall_debug(char* params[]) {
	for (int i=0; i<2; i++) {
		log(params[i], NULL);
	}
}

void syscall_handler(Register* regs) {
	log("EAX: %x\n", regs->eax);
	log("EBX: %d\n", regs->ebx);
	log("ECX: %d\n", regs->ecx);
	log("EDX: %d\n", regs->edx);
	if (regs->eax == 0) {
		// Get VBE info
		memset32(regs->ebx, (int)vbeInfo, 1);
	} else if (regs->eax == 1) {
		// Get screen
		memset32(regs->ebx, (int)get_screen(), 1);
	} else if (regs->eax == 2) {
		// Output to any terminal; send as broadcast
		send_broadcast(SCRIPT_MESSAGE, (char*)regs->ebx);
	} else if (regs->eax == 3) {
		// Receive any broadcast
		Broadcast* b = receive_broadcast();
		if (b == NULL) {
			memset32(regs->ebx, 0, 1);
		} else {
			memset32(regs->ebx, b->type, 1);
			memset32(regs->ecx, (uint32_t)b->message, 1);
		}
	} else if (regs->eax == 4) {
		printf((char*)regs->ebx);
	} else if (regs->eax == 5) {
		char ch = (char)regs->ebx;
		char string[2];
		string[0] = ch;
		string[1] = 0;
		printf(string);
	} else if (regs->eax == 6) {
		int length = regs->ecx;
		char* text = (char*)malloc(length+1);
		memcpy(text, (char*)regs->ebx, length);
		text[length] = 0;
		printf(text);
		free(text);
	} else if (regs->eax == 7) {
		int size = regs->ecx;
		char* allocation = (char*)malloc(size);
		memset32(regs->ebx, (int)allocation, 1);
	} else if (regs->eax == 8) {
		char* name = (char*)regs->ebx;
		int bufferPtr = regs->ecx;
		int idPtr = regs->edx;
		File* file = open_file(name);
		int bufferLoc = (int)file->content;
		int id = file->id;
		memset32(bufferPtr, bufferLoc, 1);
		memset32(idPtr, id, 1);
	} else if (regs->eax == 9) {
		int number = regs->ebx;
		printf("%d\n", number);
	} else if (regs->eax == 0x0A) {
		int fileId = regs->ebx;
		int infoPtr = regs->ecx;
		File* file = get_open_file(fileId);
		uint32_t info[4];
		info[0] = (int)file->path[0]; //Device ID (it means Label in Watermelon)
		info[1] = file->size; //File size
		info[2] = file->bytesPerSector; //Bytes per sector
		info[3] = file->size/512; //Block (sector) count
		memset32(infoPtr, info, 1);
	} else if (regs->eax == 0x0B) {
		int fileId = regs->ebx;
		uint32_t* info = (uint32_t*)regs->ecx;
		int ptr = info[0];
		int direction = info[1];
		int endSeekPtr = regs->edx;
		seek_file(fileId, ptr, direction, endSeekPtr);
	} else if (regs->eax == 0x0C) {
		int fileId = regs->ebx;
		char* buffer = (char*)regs->ecx;
		int len = regs->edx;
		read_file(fileId, buffer, len);
	} else if (regs->eax == 0x0D) {
		char* data = (char*)regs->ebx;
		int len = regs->ecx;
		dump(data, len);
	} else if (regs->eax == 0x0E) {
		int fileId = regs->ebx;
		int sizePtr = regs->ecx;
		File* file = get_open_file(fileId);
		memset32(sizePtr, file->id, 1);
	} else if (regs->eax == 0x0F) {
		int fileId = regs->ebx;
		int idPtr = regs->ecx;
		File* file = get_open_file(fileId);
		memset32(idPtr, (int)file->path[0], 1);
	} else if (regs->eax == 0x10) {
		int fileId = regs->ebx;
		int blkSizePtr = regs->ecx;
		File* file = get_open_file(fileId);
		memset32(blkSizePtr, file->bytesPerSector, 1);
	} else if (regs->eax == 0x11) {
		fill_rect(0, 0, 100, 100, 0xffff0000);
		flush();
	} else if (regs->eax == 0x12) {
		put_with_alpha(regs->ebx, regs->ecx, regs->edx);
	} else if (regs->eax == 0x13) {
		Window* window = (Window*)regs->ebx;
		create_window(window);
		window->buffer = (uint8_t*)malloc(sizeof(window->width*window->height*get_Bpp()));
		
	} else if (regs->eax == 0x14) {
		// Max 10 parameters
		char** argv = (char**)regs->ebx;
		log(
			argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]
		);
	}
}

/* Syscall codes (regs->eax):
00 = get vbe info
01 = get screen
02 = Output to any terminal
03 = Receive any broadcast
04 = print text when is in text mode
05 = print a single character when is in text mode
06 = print text with length when is in text mode
07 = sbrk (asking for a piece of free memory)
08 = open file
09 = print number when is in text mode
0A = fstat (get file statistic)
0B = lseek
0C = read file
0D = dump data when is in text mode
0E = get file size by file ID
0F = get device ID by file ID
10 = get block size
11 = test
12 = put pixel
13 = create window
14 = debug
*/

void call(int eax, int ebx, int ecx, int edx) {
	asm volatile("int $48"::"a"(eax),"b"(ebx),"c"(ecx),"d"(edx));
}
