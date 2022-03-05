#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <system/window.h>

#define ARIAL_FONT_SIZE 296712
#define FROM_LEFT_TOP 0
#define FROM_RIGHT_BOTTOM 1
#define CENTER_ON_SCREEN 2
#define FADE_OUT 0
#define FADE_IN 1

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

void init_graphics(VbeInfo* vbeInfo);
unsigned char* get_screen();
void set_screen(uint8_t* scr);
unsigned char* get_main_screen();
unsigned char* get_offscreen();
int get_width();
int get_height();
void flush();
void flush_area(int x, int y, int width, int height);
int get_offset(int x, int y);
void fill_rect(int x, int y, int width, int height, int color);
void put(int x, int y, int color);
void put_with_alpha(int x, int y, int color);
void put_to_screen(uint8_t* screen, int x, int y, int color);
extern "C" void flush_sse(unsigned char* screen, unsigned char* current_screen, int copy_size);
void fill_bg(int color);
extern VbeInfo* vbeInfo;
void draw_image(char* data, int x, int y, int position);
void draw_bmp_image(unsigned char* data, int x, int y);
int get_color(int x, int y);
int get_color_with_alpha(int color, int bgColor, int alpha_);
void set_font(char* fontData, int size);
void set_font_size(int size);
void draw_text(char* text, int x, int y, int color);
void transition(int type, int delta, void (*drawAtRefreshFunction)());
unsigned char* get_screen_copy();
void plot_screen(unsigned char* data);
void draw_animation(char* data, int total, int delay, int x, int y, int position, void (*drawAtRefreshFunction)());
int get_image_width(char* data);
int get_image_height(char* data);
void set_clip(int x, int y, int w, int h);
void unset_clip();
void draw_gradient(int x, int y, int width, int height, int color1, int color2, int direction);
void draw_line(int x0, int y0, int x1, int y1, int color);
void draw_circle(unsigned int centerX, unsigned int centerY, unsigned int radius, int color);
void fill_circle(unsigned int centerX, unsigned int centerY, unsigned int radius, int color);
uint8_t* copy_screen_buffer(int x, int y, int width, int height);
void copy_to_screen(uint8_t* buffer, int x, int y, int width, int height);
int get_buffer_offset(int x, int y, int bufferWidth, int bufferHeight);
void custom_copy_screen_buffer(uint8_t* src, int srcX, int srcY, int srcWidth, int srcHeight, uint8_t* dst, int dstX, int dstY, int dstWidth, int dstHeight, int copyWidth, int copyHeight);
void draw_image_to_screen(uint8_t* screen, char* data, int x, int y, int position);
unsigned char* get_windows_buffer();
void copy_screen(unsigned char* dst, unsigned char* src);
int get_bpp();
int get_Bpp();
void flush_window(Window* window);
int mix_color(int color1, int color2, int prop, int max);

#endif
