#include <stddef.h>
#include <stdint.h>
#include <wos/system.h>

void fill_rect(int x, int y, int width, int height, int color) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			sys_call(0x12, i, j, color);
		}
	}
}

int main() {
	sys_call(0x11, 0, 0, 0);
	return 0;
	Window* window = (Window*)malloc(sizeof(Window));
	window->x = 150;
	window->y = 50;
	window->width = 300;
	window->height = 200;
	window->backgroundColor = 0xff0000;
	window->borderWidth = 2;
	window->title = "Window 1";
	window->tag = 101;
	sys_call(0x13, (uint32_t)window, 0, 0);
	return 0;
}
