#ifndef TEXTFIELD_H
#define TEXTFIELD_H

class TextField {
public:
	int x;
	int y;
	int width;
	int height;
	int bgColor;
	int strokeColor;
	char* fontData;
	int fontDataSize;
	int fontSize;
	char* text;
	int textColor;
	
	TextField(int x, int y, int width, int height, int bgColor, int strokeColor, char* fontData, int fontDataSize, int fontSize, int textColor);
	
	void setText(char* text);
	
	void apply();
};

#endif
