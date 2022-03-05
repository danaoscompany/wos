#ifndef WINDOW_H
#define WINDOW_H

typedef struct Window_t {
	int x;
	int y;
	int width;
	int height;
	int bpp;
	int Bpp;
	int backgroundColor;
	uint8_t* tag;
	int prevX;
	int prevY;
	int mouseDistanceX;
	int mouseDistanceY;
	int borderWidth;
	char* title;
	uint8_t* buffer;
	void (*flush)(struct Window_t* window);
	void (*fill_rect)(struct Window_t* window, int x, int y, int width, int height, int color);
} Window;

void init_windows();
void create_window(Window* window);
void refresh_windows();
void window_fill_rect(Window* window, int x, int y, int width, int height, int color);
void window_draw_gradient(Window* window, int x, int y, int width, int height, int color1, int color2, int direction);

#endif
