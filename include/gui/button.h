#ifndef BUTTON_H
#define BUTTON_H

class Button {
public:
	char* text;
	int x;
	int y;
	int textColor;
	int width;
	int height;
	int bgColor;
	int bgColorPressed;
	int bgColorHovered;
	int textOffsetFromLeft;
	int textOffsetFromTop;
	int textSize;
	void (*clickListener)();
	bool hasHoverEffect = false;
	bool hasClickEffect = true;
	
	Button(char* text, int x, int y, int textSize, int textColor, int bgColor, int width, int height);
	
	void setHoverBgColor(int color);
	
	void setPressedBgColor(int color);
	
	void apply();
};

#endif
