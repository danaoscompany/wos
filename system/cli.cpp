#include <system.h>
#include <stdarg.h>

static char* screen = (char*)0xB8000;
static char hex_value[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
int screenPos = 0;

void print_text(char* text, int x, int y) {
	int pos = (y*80+x)*2;
	int i = 0;
	while (text[i] != 0) {
		screen[pos] = text[i];
		screen[pos+1] = 0x0F;
		i++;
		pos += 2;
	}
}

void print_number(int number, int x, int y) {
	int pos = 2*(y*80+x);
	if (number == 0) {
		screen[pos] = '0';
		screen[pos+1] = 0x0F;
		return;
	} else if (number < 0) {
		number = -number;
		screen[pos] = '-';
		screen[pos+1] = 0x0F;
		pos += 2;
	}
	pos += (numlen(number)-1)*2;
	while (number > 0) {
		screen[pos] = number%10+'0';
		screen[pos+1] = 0x0F;
		pos -= 2;
		number /= 10;
	}
}

void print_hex_number(int number, int x, int y) {
	int pos = 2*(y*80+x);
	screen[pos] = '0';
	screen[pos+1] = 0x0F;
	screen[pos+2] = 'x';
	screen[pos+3] = 0x0F;
	pos += 4;
	if (number == 0) {
		screen[pos] = '0';
		screen[pos+1] = 0x0F;
		return;
	}
	pos += (numlenhex(number)-1)*2;
	while (number > 0) {
		screen[pos] = hex_value[number%16];
		screen[pos+1] = 0x0F;
		pos -= 2;
		number /= 16;
	}
}

void print_hex_number_unsigned(unsigned int number, int x, int y) {
	int pos = 2*(y*80+x);
	screen[pos] = '0';
	screen[pos+1] = 0x0F;
	screen[pos+2] = 'x';
	screen[pos+3] = 0x0F;
	pos += 4;
	if (number == 0) {
		screen[pos] = '0';
		screen[pos+1] = 0x0F;
		return;
	}
	pos += (numlenhexunsigned(number)-1)*2;
	while (number > 0) {
		screen[pos] = hex_value[number%16];
		screen[pos+1] = 0x0F;
		pos -= 2;
		number /= 16;
	}
}

void printf(char* text, ...) {
	if (screenPos > 4000) {
		return;
	}
	va_list args;
	va_start(args, 10);
	int index = 0;
	while (text[index] != 0) {
		if (text[index] == '%') {
			if (text[index+1] == 'c') {
				char ch = va_arg(args, int);
				screen[screenPos] = ch;
				screen[screenPos+1] = 0x0F;
				screenPos += 2;
				index += 2;
			} else if (text[index+1] == 'b') {
				char b = va_arg(args, int);
				if (b) {
					printf("true");
					screenPos += 2*4;
					index += 2;
				} else {
					printf("false");
					screenPos += 2*5;
					index += 2;
				}
			} else if (text[index+1] == 's') {
				char* string = va_arg(args, char*);
				printf(string);
				screenPos += 2*strlen(string);
				index += 2;
			} else if (text[index+1] == 'd') {
				int number = va_arg(args, int);
				print_number(number, screenPos/2, 0);
				screenPos += 2*numlen(number);
				index += 2;
			} else if (text[index+1] == 'x') {
				unsigned int number = va_arg(args, unsigned int);
				print_hex_number_unsigned(number, screenPos/2, 0);
				screenPos += 4;
				screenPos += 2*numlenhexunsigned(number);
				index += 2;
			} else {
				screen[screenPos] = '%';
				screen[screenPos+1] = 0x0F;
				screenPos += 2;
				index += 1;
			}
		} else if (text[index] == '\n') {
			screenPos = (screenPos/(80*2)+1)*80*2;
			index++;
		} else {
			screen[screenPos] = text[index];
			screen[screenPos+1] = 0x0F;
			screenPos += 2;
			index += 1;
		}
	}
	va_end(args);
}

void resetf() {
	screenPos = 0;
}

void clear_screen() {
	memset(0xB8000, 0, 4000);
}

void dump(char* data, int count) {
	for (int i=0; i<count; i++) {
		screen[screenPos] = data[i];
		screen[screenPos+1] = 0x0F;
		screenPos += 2;
	}
	printf("\n");
}
