#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

struct Window_t {
	int x;
	int y;
	int width;
	int height;
	int backgroundColor;
	uint8_t* tag;
	int prevX;
	int prevY;
	int mouseDistanceX;
	int mouseDistanceY;
	int borderWidth;
	char* title;
	uint8_t* buffer;
	void (*flush)(struct Window_t window);
};

typedef struct Window_t Window;

void create_window(Window* window);

#endif
