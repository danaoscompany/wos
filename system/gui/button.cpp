#include <system.h>

Button::Button(char* text, int x, int y, int textSize, int textColor, int bgColor, int width, int height) {
	this->text = text;
	this->x = x;
	this->y = y;
	this->textColor = textColor;
	this->bgColor = bgColor;
	this->width = width;
	this->height = height;
	this->textSize = textSize;
	bgColorPressed = 0xAF3D28;
	textOffsetFromLeft = 10;
	textOffsetFromTop = 32;
	clickListener = 0;
}

void Button::apply() {
	if (get_cursor_x() > x && get_cursor_x() < x+width && get_cursor_y() > y && get_cursor_y() < y+height) {
		// Hovered
		// Make sure the button is not clicked
		if (!is_left_clicked()) {
			if (hasHoverEffect) {
				fill_rect(x, y, width, height, bgColorHovered);
				set_font_size(textSize);
				draw_text(text, x+textOffsetFromLeft, y+textOffsetFromTop, textColor);
				flush_area(x, y, width, height);
			}
		}
	} else {
		fill_rect(x, y, width, height, bgColor);
		set_font_size(textSize);
		draw_text(text, x+textOffsetFromLeft, y+textOffsetFromTop, textColor);
	}
	if (clickListener != 0) {
		if (is_left_clicked()) {
			if (get_cursor_x() > x && get_cursor_x() < x+width && get_cursor_y() > y && get_cursor_y() < y+height) {
				// Clicked
				if (hasClickEffect) {
					fill_rect(x, y, width, height, bgColorPressed);
					set_font_size(textSize);
					draw_text(text, x+textOffsetFromLeft, y+textOffsetFromTop, textColor);
				}
				clickListener();
			}
		}
	}
}

void Button::setHoverBgColor(int color) {
	bgColorHovered = color;
	hasHoverEffect = true;
}

void Button::setPressedBgColor(int color) {
	bgColorPressed = color;
	hasClickEffect = true;
}
