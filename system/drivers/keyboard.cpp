#include <system.h>

static bool key_pressed = false;
static uint8_t pressed_key = 0;
static char* _qwertzuiop = "qwertzuiop";
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";

void keyboard_handler(Register* regs) {
	uint8_t key = inportb(0x60);
	if ((key&0x80) == 0x80) {
		// Key is released
		key_pressed = true;
	} else {
		// Key is pressed
		pressed_key = key;
	}
}

bool isKeyPressed() {
	if (key_pressed) {
		key_pressed = false;
		return true;
	}
	return false;
}

void waitForKey() {
	while (!key_pressed) {
		wait();
	}
	key_pressed = false;
}

int getPressedKey() {
	char key = pressed_key;
	if (key == 0x1C) return '\n';
	if (key == 0x39) return ' ';
	if (key == 0xE) return '\r';
	if (key == 0xB4) return '.';
	if (key == 0xB5) return '/';
	if (key == 0x29) return '0';
	if (key >= 0x2 && key <= 0xA)
		return _num[key - 0x2];
	if (key >= 0x10 && key <= 0x1C)
	{
		return _qwertzuiop[key - 0x10];
	}
	else if (key >= 0x1E && key <= 0x26)
	{
		return _asdfghjkl[key - 0x1E];
	}
	else if (key >= 0x2C && key <= 0x32)
	{
		return _yxcvbnm[key - 0x2C];
	}
	return 0;
}

void init_keyboard() {
	irq_set_handler(1, keyboard_handler);
}
