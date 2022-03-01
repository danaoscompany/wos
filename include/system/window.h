#ifndef WINDOW_H
#define WINDOW_H

typedef struct {
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
} Window;

void init_windows();
void create_window(Window* window);
void refresh_windows();

#endif
