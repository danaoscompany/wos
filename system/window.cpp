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
	windows[nextFreeWindowIndex] = window;
	nextFreeWindowIndex++;
	windowsNeedRedrawn = true;
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
								fill_rect(window->x, window->y, window->width, window->height, window->backgroundColor);
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
				fill_rect(window->x, window->y, window->width, window->height, window->backgroundColor);
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
				fill_rect(window->x, window->y, window->width, window->height, window->backgroundColor);
			}
			window->prevX = window->x;
			window->prevY = window->y;
		}
	}
}
