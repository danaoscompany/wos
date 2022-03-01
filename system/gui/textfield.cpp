#include <system.h>

TextField::TextField(int x, int y, int width, int height, int bgColor, int strokeColor, char* fontData, int fontDataSize, int fontSize, int textColor) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->bgColor = bgColor;
	this->strokeColor = strokeColor;
	this->fontData = fontData;
	this->fontDataSize = fontDataSize;
	this->fontSize = fontSize;
	this->textColor = textColor;
}

void TextField::setText(char* text) {
	this->text = text;
}

void TextField::apply() {
	fill_rect(x, y, width, height, strokeColor);
	fill_rect(x+2, y+2, width-4, height-4, bgColor);
	//set_font_data(fontData, fontDataSize);
	set_font_size(fontSize);
	draw_text(text, x+10, y+10, textColor);
}
