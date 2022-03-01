#ifndef MOUSE_H
#define MOUSE_H

typedef enum {
	LEFT_CLICK   = 0x01,
	RIGHT_CLICK  = 0x02,
	MIDDLE_CLICK = 0x04
} mouse_click_t;

typedef struct {
	uint32_t magic;
	char x_difference;
	char y_difference;
	mouse_click_t buttons;
} mouse_device_packet_t;

int get_cursor_x();
int get_cursor_y();
void init_mouse();
bool is_left_clicked();
bool is_right_clicked();
bool is_middle_clicked();
bool is_left_pressed();
void draw_cursor();
bool is_clicked(int x1, int y1, int x2, int y2);
void refresh_mouse();
bool is_dragging();
bool is_mouse_moving();

#endif
