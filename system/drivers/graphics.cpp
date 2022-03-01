#include <system.h>

static unsigned char* screen;
static unsigned char* offscreen;
static unsigned char* windows_buffer;
static unsigned char* current_screen;
static int screen_width;
static int screen_height;
static int bpp;
static int Bpp;
static int copy_size;
static bool use_sse = false;
static bool mtrr_enabled = false;
VbeInfo* vbeInfo;
static int current_font_data;
static int current_font_size;
static int current_word_size;
static int clip_x, clip_y, clip_width, clip_height;
static bool clip_set = false;

void init_graphics(VbeInfo* _vbeInfo) {
	vbeInfo = _vbeInfo;
	screen = (unsigned char*)vbeInfo->phys;
	screen_width = (int)vbeInfo->x_res;
	screen_height = (int)vbeInfo->y_res;
	bpp = (int)vbeInfo->bits_per_pixel;
	Bpp = bpp/8+((bpp%8)?1:0);
	offscreen = (unsigned char*)malloc(screen_width*screen_height*Bpp);
	windows_buffer = (unsigned char*)malloc(screen_width*screen_height*Bpp);
	copy_size = screen_width*screen_height*Bpp;
	current_screen = offscreen;
	// Checking for SSE. If it is available, then use it.
	cpuid(1);
	int eax = cpuid_eax;
	int edx = cpuid_edx;
	if ((edx&2000000)!=0) {
		use_sse = true;
		int cr0 = read_cr(0);
		cr0 &= 0x4;
		write_cr(0, cr0);
		cr0 = read_cr(0);
		cr0 |= 0x1;
		write_cr(0, cr0);
		int cr4 = read_cr(4);
		cr4 |= 0x200;
		write_cr(4, cr4);
		cr4 = read_cr(4);
		cr4 |= 0x400;
		write_cr(4, cr4);
	}
	// Checking for MTRR. If it is available, than enable it.
	cpuid(1);
	eax = cpuid_eax;
	edx = cpuid_edx;
	if ((edx&0x1000)!=0) {
		// MTRR is supported. Now let's check wheter Write Combining (WC) memory-caching supported.
		read_msr(0xFE);
		eax = msr_eax;
		edx = msr_edx;
		if ((eax&0x400)!=0) {
			// Write Combining is supported. Enable MTRR...
			write_msr(0x2FF, 0xC01, 0x0);
			// Now MTRR is enabled!
			mtrr_enabled = true;
		}
	}
}

unsigned char* get_screen() {
	return current_screen;
}

unsigned char* get_main_screen() {
	return screen;
}

unsigned char* get_offscreen() {
	return offscreen;
}

unsigned char* get_windows_buffer() {
	return windows_buffer;
}

void set_screen(uint8_t* scr) {
	current_screen = scr;
}

void flush() {
	draw_cursor();
	if (use_sse) {
		flush_sse(screen, current_screen, copy_size);
	} else {
		memcpy(screen, current_screen, copy_size);
	}
}

void flush_area(int x, int y, int width, int height) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			int offset = get_offset(i, j);
			if (bpp == 24) {
				screen[offset] = current_screen[offset];
				screen[offset+1] = current_screen[offset+1];
				screen[offset+2] = current_screen[offset+2];
			}
		}
	}
}

int get_offset(int x, int y) {
	//return asm_get_offset(x, y);
	return (y*screen_width+x)*Bpp;
}

int get_buffer_offset(int x, int y, int bufferWidth, int bufferHeight) {
	return (y*bufferWidth+x)*Bpp;
}

void put(int x, int y, int color) {
	if (clip_set) {
		if (x < clip_x || x >= clip_x+clip_width || y < clip_y || y >= clip_y+clip_height) return;
	} else {
		if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
	}
	int pos = get_offset(x, y);
	switch (bpp) {
		case 24:
			current_screen[pos] = color&0xFF;
			current_screen[pos+1] = color>>8&0xFF;
			current_screen[pos+2] = color>>16&0xFF;
			break;
	}
}

void put_to_screen(uint8_t* screen, int x, int y, int color) {
	if (clip_set) {
		if (x < clip_x || x >= clip_x+clip_width || y < clip_y || y >= clip_y+clip_height) return;
	} else {
		if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
	}
	int pos = get_offset(x, y);
	switch (bpp) {
		case 24:
			screen[pos] = color&0xFF;
			screen[pos+1] = color>>8&0xFF;
			screen[pos+2] = color>>16&0xFF;
			break;
	}
}

void set_clip(int x, int y, int w, int h) {
	clip_x = x;
	clip_y = y;
	clip_width = w;
	clip_height = h;
	clip_set = true;
}

void unset_clip() {
	clip_set = false;
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

int get_color_of_screen(int x, int y, unsigned char* scr) {
	// scr must be scr+1! //A serious bug in GCC
	//scr = (unsigned char*)((uint32_t)scr-1);
	if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
		return 0;
	}
	int pos = get_offset(x, y);
	switch (bpp) {
		case 24:
			int red = (int)scr[pos+2];
			int green = (int)scr[pos+1];
			int blue = (int)scr[pos];
			int color = ((red<<16)|(green<<8)|blue);
			return color;
	}
	return 0;
}

int get_width() {
	return screen_width;
}

int get_height() {
	return screen_height;
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

void fill_rect(int x, int y, int width, int height, int color) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			put(i, j, color);
		}
	}
}

void fill_rect_with_alpha(int x, int y, int width, int height, int color, int alpha) {
	for (int j=y; j<y+height; j++) {
		for (int i=x; i<x+width; i++) {
			int bgColor = get_color(i, j);
			color = get_color_with_alpha(color, bgColor, alpha);
			put(i, j, color);
		}
	}
}

void fill_bg(int color) {
	fill_rect(0, 0, screen_width, screen_height, color);
}

void fill_bg_with_alpha(int color, int alpha) {
	fill_rect_with_alpha(0, 0, screen_width, screen_height, color, alpha);
}

uint8_t* copy_screen_buffer(int x, int y, int width, int height) {
	uint8_t* buffer = (uint8_t*)malloc(screen_width * screen_height * Bpp);
	for (int j = 0; j < height; j++) {
		memcpy(buffer + (j * Bpp * width), screen + get_offset(x, y + j), width * Bpp);
	}
	return buffer;
}

void draw_bmp_image(unsigned char* data, int x, int y) {
	int dataStart = charstoint(data[13], data[12], data[11], data[10]);
	int width = charstoint(data[21], data[20], data[19], data[18]);
	int height = charstoint(data[25], data[24], data[23], data[22]);
	int bpp = charstoshort(data[29], data[28]);
	int dataSize = charstoint(data[37], data[36], data[35], data[34]);
	int pos = dataStart;
	if (bpp == 24) {
		for (int j=y+height-1; j>=y; j--) {
			for (int i=x; i<x+width; i++) {
				int blue = (int)data[pos];
				int green = (int)data[pos+1];
				int red = (int)data[pos+2];
				int color = (red<<16)|(green<<8)|blue;
				put(i, j, color);
				pos += 3;
			}
			if (((pos-dataStart)%4)!=0) {
				int rowCount = pos-dataStart;
				rowCount = (rowCount/4+1)*4;
				pos = dataStart+rowCount;
			}
		}
	} else if (bpp == 32) {
		for (int j=y+height-1; j>=y; j--) {
			for (int i=x; i<x+width; i++) {
				int alpha = (int)data[pos];
				int blue = (int)data[pos+1];
				int green = (int)data[pos+2];
				int red = (int)data[pos+3];
				int color = (red<<16)|(green<<8)|blue;
				color = get_color_with_alpha(color, get_color(i, j), alpha);
				put(i, j, color);
				pos += 4;
			}
		}
	}
}

void draw_bmp_image_to_screen(uint8_t* screen, unsigned char* data, int x, int y) {
	int dataStart = charstoint(data[13], data[12], data[11], data[10]);
	int width = charstoint(data[21], data[20], data[19], data[18]);
	int height = charstoint(data[25], data[24], data[23], data[22]);
	int bpp = charstoshort(data[29], data[28]);
	int dataSize = charstoint(data[37], data[36], data[35], data[34]);
	int pos = dataStart;
	if (bpp == 24) {
		for (int j=y+height-1; j>=y; j--) {
			for (int i=x; i<x+width; i++) {
				int blue = (int)data[pos];
				int green = (int)data[pos+1];
				int red = (int)data[pos+2];
				int color = (red<<16)|(green<<8)|blue;
				put_to_screen(screen, i, j, color);
				pos += 3;
			}
			if (((pos-dataStart)%4)!=0) {
				int rowCount = pos-dataStart;
				rowCount = (rowCount/4+1)*4;
				pos = dataStart+rowCount;
			}
		}
	} else if (bpp == 32) {
		for (int j=y+height-1; j>=y; j--) {
			for (int i=x; i<x+width; i++) {

				int alpha = (int)data[pos];
				int blue = (int)data[pos+1];
				int green = (int)data[pos+2];
				int red = (int)data[pos+3];
				int color = (red<<16)|(green<<8)|blue;
				color = get_color_with_alpha(color, get_color(i, j), alpha);
				put_to_screen(screen, i, j, color);
				pos += 4;
			}
		}
	}
}

int get_image_width(char* data) {
	// Checking file type
	if (data[0] == 'B' && data[1] == 'M') {
		// This is BMP file
		int width = charstouint(data[21], data[20], data[19], data[18]);
		return width;
	}
	return 0;
}

int get_image_height(char* data) {
	// Checking file type
	if (data[0] == 'B' && data[1] == 'M') {
		// This is BMP file
		int height = charstoint(data[25], data[24], data[23], data[22]);
		return height;
	}
	return 0;
}

void draw_image(char* data, int x, int y, int position) {
	if (position == FROM_RIGHT_BOTTOM) {
		x = get_width()-x-get_image_width(data);
		y = get_height()-y-get_image_height(data);
	} else if (position == CENTER_ON_SCREEN) {
		x = (get_width()/2)-(get_image_width(data)/2)+x;
		y = (get_height()/2)-(get_image_height(data)/2)+y;
	}
	// Checking file type
	if (data[0] == 'B' && data[1] == 'M') {
		// This is BMP file
		draw_bmp_image((unsigned char*)data, x, y);
	}
}

void draw_image_to_screen(uint8_t* screen, char* data, int x, int y, int position) {
	if (position == FROM_RIGHT_BOTTOM) {
		x = get_width()-x-get_image_width(data);
		y = get_height()-y-get_image_height(data);
	} else if (position == CENTER_ON_SCREEN) {
		x = (get_width()/2)-(get_image_width(data)/2)+x;
		y = (get_height()/2)-(get_image_height(data)/2)+y;
	}
	// Checking file type
	if (data[0] == 'B' && data[1] == 'M') {
		// This is BMP file
		draw_bmp_image_to_screen(screen, (unsigned char*)data, x, y);
	}
}

void set_font(char* fontData, int size) {
	current_font_data = fontData;
	current_font_size = size;
}

void set_font_size(int size) {
	current_word_size = size;
}

void draw_text(char* text, int x, int y, int color) {
	char** argv = (char**)malloc(sizeof(int)*7);
	argv[0] = (char*)&arial_font;
	argv[1] = current_font_size;
	argv[2] = current_word_size;
	argv[3] = text;
	argv[4] = x;
	argv[5] = y;
	argv[6] = color;
	run_app((char*)&text_drawer, 7, argv);
}

void plot_screen(unsigned char* data) {
	if (use_sse) {
		flush_sse(current_screen, data, copy_size);
	} else {
		memcpy(current_screen, data, copy_size);
	}
}

void copy_screen(unsigned char* dst, unsigned char* src) {
	if (use_sse) {
		flush_sse(dst, src, copy_size);
	} else {
		memcpy(dst, src, copy_size);
	}
}

void copy_to_screen(uint8_t* buffer, int x, int y, int width, int height) {
	for (int j = 0; j < height; j++) {
		memcpy(screen + get_offset(x, y + j), buffer + (j * width * Bpp), width * Bpp);
	}
}

void custom_copy_screen_buffer(uint8_t* src, int srcX, int srcY, int srcWidth, int srcHeight, uint8_t* dst, int dstX, int dstY, int dstWidth, int dstHeight, int copyWidth, int copyHeight) {
	for (int j = 0; j < copyHeight; j++) {
		memcpy(dst + get_buffer_offset(dstX, dstY + j, dstWidth, dstHeight), src + get_buffer_offset(srcX, srcY + j, srcWidth, srcHeight), copyWidth * Bpp);
	}
}

unsigned char* get_screen_copy() {
	unsigned char* data = (unsigned char*)malloc(copy_size);
	umemcpy(data, current_screen, copy_size);
	return data;
}

void transition(int type, int delta, void (*drawAtRefreshFunction)()) {
	// Example values: type = FADE_OUT or FADE_IN
	int alpha = (type==FADE_IN)?255:0;
	int target = (type==FADE_IN)?0:255;
	while ((type==FADE_IN)?alpha > target:alpha < target) {
		if (drawAtRefreshFunction != NULL) {
			drawAtRefreshFunction();
		}
		for (int j=0; j<get_height(); j++) {
			for (int i=0; i<get_width(); i++) {
				int color = get_color_of_screen(i, j, current_screen);
				color = get_color_with_alpha(0x000000, color, alpha);
				put(i, j, color);
			}
		}
		if (type == FADE_IN) {
			alpha -= delta;
		} else {
			alpha += delta;
		}
		flush();
	}
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
  for(;;){
    put(x0, y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void draw_circle(unsigned int x0, unsigned int y0, unsigned int radius,int color) {
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
 
    put(x0, y0 + radius, color);
    put(x0, y0 - radius, color);
    put(x0 + radius, y0, color);
    put(x0 - radius, y0, color);
 
    while(x < y) 
    {
        if(f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;    
        put(x0 + x, y0 + y, color);
        put(x0 - x, y0 + y, color);
        put(x0 + x, y0 - y, color);
        put(x0 - x, y0 - y, color);
        put(x0 + y, y0 + x, color);
        put(x0 - y, y0 + x, color);
        put(x0 + y, y0 - x, color);
        put(x0 - y, y0 - x, color);
    }
}

void fill_circle(unsigned int x0, unsigned int y0, unsigned int radius,int color) {
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
    draw_line(x0, y0-radius, x0, y0+radius, color);
    draw_line(x0-radius, y0, x0+radius, y0, color);
 
    while(x < y) 
    {
        if(f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;    
        draw_line(x0 - x, y0 + y, x0 + x, y0 + y, color);
        draw_line(x0-x, y0-y, x0+x, y0-y, color);
        draw_line(x0-y, y0+x, x0+y, y0+x, color);
        draw_line(x0-y, y0-x, x0+y, y0-x, color);
    }
}

static int mix_color(int color1, int color2, int prop, int max) {
	int red = (((color1>>16)&0xFF)*prop+((color2>>16)&0xFF)*(max-prop))/max;
	int green = (((color1>>8)&0xFF)*prop+((color2>>8)&0xFF)*(max-prop))/max;
	int blue = ((color1&0xFF)*prop+(color2&0xFF)*(max-prop))/max;
	return (red<<16)|(green<<8)|blue;
}

void draw_gradient(int x, int y, int width, int height, int color1, int color2, int direction) {
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
			draw_line(x+i, y, x+i, y+height-1, color);
		} else if (direction == 1) {
			draw_line(x, y+i, x+width-1, y+i, color);
		}
	}
}

void draw_animation(char* data, int total, int delay, int x, int y, int position, void (*drawAtRefreshFunction)()) {
	int i = 0;
	int clipWidth = get_image_width(data)/total;
	int clipHeight = get_image_height(data);
	if (position == FROM_RIGHT_BOTTOM) {
		x = get_width()-x-get_image_width(data);
		y = get_height()-y-get_image_height(data);
	} else if (position == CENTER_ON_SCREEN) {
		x = (get_width()/2)-(clipWidth/2)+x;
		y = (get_height()/2)-(clipHeight/2)+y;
	}
	int imageX = x;
	int imageY = y;
	while (i < total) {
		if (drawAtRefreshFunction != NULL) {
			drawAtRefreshFunction();
		}
		set_clip(x, y, clipWidth, clipHeight);
		// Checking file type
		if (data[0] == 'B' && data[1] == 'M') {
			// This is BMP file
			draw_bmp_image(data, imageX, imageY);
		}
		unset_clip();
		flush();
		imageX -= clipWidth;
		i++;
		sleep(delay);
	}
}
