#include <system.h>

#define WINDOWS_COUNT 100

Window** windows;
int nextFreeWindowIndex = 0;
bool windowsNeedRedrawn = false;
bool windowsMovedNeedRedrawn = false;
bool mouseIsMoving = false;
int movingWindowIndex = 0;
bool needToRedrawPrevWindows = false;

void init_windows() {
	windows = (Window**)malloc(sizeof(Window*)*WINDOWS_COUNT);
}

void create_window(Window* window) {
	if (window->bpp == 0) {
		window->bpp = get_bpp();
	}
	if (window->Bpp == 0) {
		window->Bpp = get_Bpp();
	}
	windows[nextFreeWindowIndex] = window;
	nextFreeWindowIndex++;
	windowsNeedRedrawn = true;
}

void draw_window(Window* window) {
	// Draw border
	fill_rect(window->x, window->y, window->borderWidth, window->height, 0xffdbdbdb);
	fill_rect(window->x, window->y, window->width, window->borderWidth, 0xffdbdbdb);
	fill_rect(window->x, window->y+window->height-window->borderWidth, window->width, window->borderWidth, 0xff010101);
	fill_rect(window->x+window->width-window->borderWidth, window->y, window->borderWidth, window->height, 0xff010101);
	fill_rect(window->x+window->borderWidth, window->y+window->borderWidth, window->borderWidth, window->height-window->borderWidth*2, 0xffffffff);
	fill_rect(window->x+window->borderWidth, window->y+window->borderWidth, window->width-window->borderWidth*2, window->borderWidth, 0xffffffff);
	fill_rect(window->x+window->borderWidth, window->y+window->height-window->borderWidth*2, window->width-window->borderWidth*2, window->borderWidth, 0xffb0b0b0);
	fill_rect(window->x+window->width-window->borderWidth*2, window->y+window->borderWidth, window->borderWidth, window->height-window->borderWidth*2, 0xffb0b0b0);
	// Draw background
	fill_rect(window->x+window->borderWidth*2, window->y+window->borderWidth*2, window->width-window->borderWidth*4, window->height-window->borderWidth*4, 0xffbfbfbf);
	// Draw title bar
	if (window->title != NULL) {
		draw_gradient(window->x+window->borderWidth*3, window->y+window->borderWidth*3, window->width-(2*window->borderWidth*3), 25, 0xff01017b, 0xff1085d2, 0);
	}
	//draw_text(window->title, 30, 25, 0xffffffff);
}

void refresh_windows() {
	int mouseX = get_cursor_x();
	int mouseY = get_cursor_y();
	if (!mouseIsMoving && is_left_clicked()) {
		for (int i=nextFreeWindowIndex-1; i>=0; i--) {
			Window* window = windows[i];
			if (mouseX >= window->x && mouseX < (window->x+window->width) && mouseY >= window->y && mouseY < (window->y+window->height)) {
				if (i != (nextFreeWindowIndex-1)) {
					Window** tmpWindows = (Window**)malloc(sizeof(Window*)*WINDOWS_COUNT);
					for (int j=0; j<nextFreeWindowIndex; j++) {
						tmpWindows[j] = windows[j];
					}
					for (int j=i; j<nextFreeWindowIndex-1; j++) {
						tmpWindows[j] = windows[j+1];
					}
					window->mouseDistanceX = mouseX-window->x;
					window->mouseDistanceY = mouseY-window->y;
					tmpWindows[nextFreeWindowIndex-1] = window;
					Window** prevWindows = windows;
					windows = tmpWindows;
					free(prevWindows);
					windowsNeedRedrawn = true;
				} else {
					if (is_mouse_moving()) {
						movingWindowIndex = i;
						mouseIsMoving = true;
						set_screen(get_offscreen());
						draw_image((char*)&wallpaper, 0, 0, 0);
						fill_rect(0, 570, 800, 30, 0x000000);
						draw_image((char*)&start_icon, 0, 571, 0);
						draw_image((char*)&file_manager_icon, 45, 573, 0);
						draw_image((char*)&shutdown_icon, 770, 573, 0);
						for (int i=0; i<nextFreeWindowIndex; i++) {
							if (i != movingWindowIndex) {
								Window* window = windows[i];
								draw_window(window);
							}
						}
						flush();
						set_screen(get_main_screen());
					}
				}
				break;
			}
		}
	}
	if (mouseIsMoving) {
		if (is_left_pressed()) {
			ack();
			Window* window = windows[movingWindowIndex];
			window->x = mouseX-window->mouseDistanceX;
			window->y = mouseY-window->mouseDistanceY;
			windowsNeedRedrawn = true;
		} else {
			mouseIsMoving = false;
			windowsNeedRedrawn = true;
		}
	}
	if (windowsNeedRedrawn) {
		windowsNeedRedrawn = false;
		if (!mouseIsMoving) {
			set_screen(get_offscreen());
			for (int i=0; i<nextFreeWindowIndex; i++) {
				Window* window = windows[i];
				draw_window(window);
				flush_window(window);
			}
			flush();
			set_screen(get_main_screen());
		}
		for (int i=0; i<nextFreeWindowIndex; i++) {
			Window* window = windows[i];
			if ((mouseIsMoving && movingWindowIndex == i) || !mouseIsMoving) {
				if (window->prevX != window->x || window->prevY != window->y) {
					if (window->x >= window->prevX && window->y >= window->prevY) {
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->prevY, get_width(), get_height(), get_main_screen(), window->prevX, window->prevY, get_width(), get_height(), window->x-window->prevX, window->height);
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->prevY, get_width(), get_height(), get_main_screen(), window->prevX, window->prevY, get_width(), get_height(), window->width, window->y-window->prevY);
					} else if (window->x < window->prevX && window->y > window->prevY) {
						custom_copy_screen_buffer(get_offscreen(), window->x+window->width, window->prevY, get_width(), get_height(), get_main_screen(), window->x+window->width, window->prevY, get_width(), get_height(), ((window->prevX+window->width)-(window->x+window->width))*10, window->height);
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->prevY-((window->y-window->prevY)*10), get_width(), get_height(), get_main_screen(), window->prevX, window->prevY-((window->y-window->prevY)*10), get_width(), get_height(), window->width, (window->y-window->prevY)*10);
					} else if (window->x < window->prevX && window->y < window->prevY) {
						custom_copy_screen_buffer(get_offscreen(), window->x+window->width, window->prevY, get_width(), get_height(), get_main_screen(), window->x+window->width, window->prevY, get_width(), get_height(), (window->prevX+window->width)-(window->x+window->width), window->height);
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->y+window->height, get_width(), get_height(), get_main_screen(), window->prevX, window->y+window->height, get_width(), get_height(), window->width, (window->prevY+window->height)-(window->y+window->height));
					} else if (window->x >= window->prevX && window->y < window->prevY) {
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->prevY, get_width(), get_height(), get_main_screen(), window->prevX, window->prevY, get_width(), get_height(), window->x-window->prevX, window->height);
						custom_copy_screen_buffer(get_offscreen(), window->prevX, window->y+window->height, get_width(), get_height(), get_main_screen(), window->prevX, window->y+window->height, get_width(), get_height(), window->width, (window->prevY+window->height)-(window->y+window->height));
					}
				}
			}
			int distanceX = mouseX-window->x;
			int distanceY = mouseY-window->y;
			if (mouseIsMoving && movingWindowIndex == i) {
				draw_window(window);
			}
			window->prevX = window->x;
			window->prevY = window->y;
		}
	}
}

int window_get_offset(Window* window, int x, int y) {
	return (y*window->width+x)*window->Bpp;
}

int window_get_color(Window* window, int x, int y) {
	if (x < 0 || x >= window->width || y < 0 || y >= window->height) {
		return 0;
	}
	int pos = window_get_offset(window, x, y);
	switch (window->bpp) {
		case 24:
			int red = (int)window->buffer[pos+2];
			int green = (int)window->buffer[pos+1];
			int blue = (int)window->buffer[pos];
			int color = ((red<<16)|(green<<8)|blue);
			return color;
	}
	return 0;
}

void window_put(Window* window, int x, int y, int _color) {
	int pos = window_get_offset(window, x, y);
	int color = get_color_with_alpha(_color, window_get_color(window, x, y), (int)((_color>>24)&0xFF));
	switch (window->bpp) {
		case 24:
			window->buffer[pos] = color&0xFF;
			window->buffer[pos+1] = color>>8&0xFF;
			window->buffer[pos+2] = color>>16&0xFF;
			break;
	}
}

void window_fill_rect(Window* window, int x, int y, int width, int height, int color) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			window_put(window, i, j, color);
		}
	}
}

void window_draw_line(Window* window, int x0, int y0, int x1, int y1, int color) {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
  for(;;) {
    window_put(window, x0, y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void window_draw_gradient(Window* window, int x, int y, int width, int height, int color1, int color2, int direction) {
	// direction = 0 (RIGHT) 1 (DOWN)
	int max;
	if (direction == 0) {
		max = width;
	} else if (direction == 1) {
		max = height;
	}
	for (int i=0; i<max; i++) {
		int color = mix_color(color1, color2, max*(max-1-i)/(max-1), max);
		if (direction == 0) {
			window_draw_line(window, x+i, y, x+i, y+height-1, color);
		} else if (direction == 1) {
			window_draw_line(window, x, y+i, x+width-1, y+i, color);
		}
	}
}
