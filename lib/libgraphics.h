#ifndef LIBGRAPHICS_H
#define LIBGRAPHICS_H

#define FONT_ARIAL 1
#define FONT_ARIAL_UNDERLINED 2
#define FONT_BEFOLK_SCRIPT 3

typedef struct {
	short mode_attrib;
	char win_a_attrib;
	char win_b_attrib;
	short win_gran;
	short win_size;
	short win_a_seg;
	short win_b_seg;
	int win_func_ptr;
	short bytes_per_scanline;
	short x_res;
	short y_res;
	char x_char_size;
	char y_char_size;
	char num_of_planes;
	char bits_per_pixel;
	char num_of_banks;
	char mem_model;
	char bank_size;
	char num_of_image_pages;
	char rsv0;
	char red_mask_size;
	char red_field_pos;
	char green_mask_size;
	char green_field_pos;
	char blue_mask_size;
	char blue_field_pos;
	char rsv_mask_size;
	char rsv_field_pos;
	char direct_color_mode_info;
	int phys;
	int rsv1;
	short rsv2;
	short lin_bytes_per_scanline;
	char bank_number_of_image_pages;
	char lin_number_of_image_pages;
	char lin_red_mask_size;
	char lin_red_field_pos;
	char lin_green_mask_size;
	char lin_green_field_pos;
	char lin_blue_mask_size;
	char lin_blue_field_pos;
	char lin_rsv_mask_size;
	char lin_rsv_field_pos;
	int max_pixel_clock;
	char rsv3[189];
} VbeInfo;

void myFunc();
unsigned char* get_screen();
int get_width();
int get_height();
void fill_rect(int x, int y, int width, int height, int color);
void fill_bg(int color);
void set_font(int font);
void put(int x, int y, int color);
int get_color(int x, int y);
int get_color_with_alpha(int color, int bgColor, int alpha_);
void set_font_size(int size);
void draw_text(char* text, int x, int y, int color);
void draw_line(int x0, int y0, int x1, int y1, int color);
void set_font_data(char* data, int size);

#endif
