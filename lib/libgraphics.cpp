#include <libsystem.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftglyph.h>

static VbeInfo* vbeInfo;
static unsigned char* screen;
static unsigned char* current_screen;
static int screen_width;
static int screen_height;
static int bpp;
static int Bpp;
static int copy_size;
static bool use_sse = false;
static bool mtrr_enabled = false;
static char* current_font_data;
static int current_font_size;
static int current_word_size;
static FT_Library ft_lib;
static FT_Face ft_face;

void danaos_init_all() {
	int addr;
	call(0, &addr, 0, 0);
	vbeInfo = (VbeInfo*)addr;
	screen_width = (int)vbeInfo->x_res;
	screen_height = (int)vbeInfo->y_res;
	bpp = (int)vbeInfo->bits_per_pixel;
	Bpp = bpp/8+((bpp%8)?1:0);
	int screenAddr;
	call(1, &screenAddr, 0, 0);
	screen = (unsigned char*)screenAddr;
	current_screen = screen;
	FT_Init_FreeType(&ft_lib);
}

void danaos_deinit() {
	FT_Done_FreeType(ft_lib);
}

unsigned char* get_screen() {
	return screen;
}

int get_width() {
	return screen_width;
}

int get_height() {
	return screen_height;
}

int get_offset(int x, int y) {
	return (y*screen_width+x)*Bpp;
}

void put(int x, int y, int color) {
	if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
	int pos = get_offset(x, y);
	switch (bpp) {
		case 24:
			current_screen[pos] = color&0xFF;
			current_screen[pos+1] = color>>8&0xFF;
			current_screen[pos+2] = color>>16&0xFF;
			break;
	}
}

void fill_rect(int x, int y, int width, int height, int color) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			put(i, j, color);
		}
	}
}

void fill_bg(int color) {
	fill_rect(0, 0, screen_width, screen_height, color);
}

int get_color(int x, int y) {
	if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
		return 0;
	}
	int pos = get_offset(x, y);
	switch (bpp) {
		case 24:
			int red = (int)current_screen[pos+2];
			int green = (int)current_screen[pos+1];
			int blue = (int)current_screen[pos];
			int color = ((red<<16)|(green<<8)|blue);
			return color;
	}
	return 0;
}

int get_color_with_alpha(int color, int bgColor, int alpha_) {
	double alpha = (double)alpha_/255;
	int red = color>>16&0xFF;
	int green = color>>8&0xFF;
	int blue = color&0xFF;
	int bgRed = bgColor>>16&0xFF;
	int bgGreen = bgColor>>8&0xFF;
	int bgBlue = bgColor&0xFF;
	int outRed = alpha*(double)red+(1-alpha)*(double)bgRed;
	int outGreen = alpha*(double)green+(1-alpha)*(double)bgGreen;
	int outBlue = alpha*(double)blue+(1-alpha)*(double)bgBlue;
	return (outRed<<16)|(outGreen<<8)|outBlue;
}

void set_font_size(int size) {
	current_word_size = size;
}

void set_font(int font) {
	if (font == FONT_ARIAL) {
		current_font_data = (char*)&arial_font;
		current_font_size = 296712;
	} else if (font == FONT_BEFOLK_SCRIPT) {
		current_font_data = (char*)&befolk_script_font;
		current_font_size = 69092;
	}
}

void set_font_data(char* data, int size) {
	current_font_data = data;
	current_font_size = size;
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
	for(;;){
    	put(x0, y0, color);
    	if (x0==x1 && y0==y1) break;
    	e2 = err;
    	if (e2 >-dx) {
    		err -= dy; x0 += sx;
    	}
    	if (e2 < dy) {
    		err += dx; y0 += sy;
    	}
  	}
}

static void draw_font_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color) {
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;
	for (i = x, p = 0; i < x_max; i++, p++) {
    	for (j = y, q = 0; j < y_max; j++, q++) {
    		if (i < 0 || j < 0 || i >= 800 || j >= 600)
        		continue;
    		int alpha = (int)bitmap->buffer[q * bitmap->width + p];
    		int c = get_color_with_alpha(color, get_color(i, j), alpha);
    		put(i, j, c);
    	}
  	}
}

void draw_text(char* text, int x, int y, int color) {
	FT_Error err = FT_New_Memory_Face(ft_lib, current_font_data, current_font_size, 0, &ft_face);
	if (err) return;
	err = FT_Set_Char_Size(ft_face, 0, current_word_size*64, 300, 300);
	if (err) return;
	err = FT_Set_Pixel_Sizes(ft_face, 0, current_word_size);
	int length = strlen(text);
	int totalWidth = 0;
	int prev_x = x;
	int prev_y = y;
	for (int i=0; i<length; i++) {
		int glyphIndex = FT_Get_Char_Index(ft_face, text[i]);
		err = FT_Load_Glyph(ft_face, glyphIndex, FT_LOAD_DEFAULT);
		if (err) continue;
		err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);
		if (err) continue;
		FT_GlyphSlot slot = ft_face->glyph;
		draw_font_bitmap(&slot->bitmap, x+slot->bitmap_left, y-slot->bitmap_top, color);
		FT_Glyph glyph;
		FT_Get_Glyph(slot, &glyph);
		FT_BBox bbox;
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
		FT_Glyph_Metrics metrics = ft_face->glyph->metrics;
		totalWidth += (metrics.horiBearingX-x+(bbox.xMax-bbox.xMin));
		x += slot->advance.x>>6;
		y += slot->advance.y>>6;
	}
	draw_line(prev_x, prev_y, prev_x-totalWidth, prev_y+30, 0x000000);
	FT_Done_Face(ft_face);
}

int get_text_width(char* text) {
	int x = 0;
	int y = 0;
	FT_Error err = FT_New_Memory_Face(ft_lib, current_font_data, current_font_size, 0, &ft_face);
	if (err) return;
	err = FT_Set_Char_Size(ft_face, 0, current_word_size*64, 300, 300);
	if (err) return;
	err = FT_Set_Pixel_Sizes(ft_face, 0, current_word_size);
	int length = strlen(text);
	int totalWidth = 0;
	for (int i=0; i<length; i++) {
		int glyphIndex = FT_Get_Char_Index(ft_face, text[i]);
		err = FT_Load_Glyph(ft_face, glyphIndex, FT_LOAD_DEFAULT);
		if (err) continue;
		err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);
		if (err) continue;
		FT_GlyphSlot slot = ft_face->glyph;
		x += slot->advance.x>>6;
		y += slot->advance.y>>6;
		if (i == length-1) {
			totalWidth = x+slot->bitmap_left;
		}
	}
	FT_Done_Face(ft_face);
	return totalWidth;
}
