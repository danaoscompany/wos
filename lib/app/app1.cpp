#include <libsystem.h>

int main(int argc, char** argv) {
	danaos_init_all();
	set_font(FONT_ARIAL);
	set_font_size(50);
	draw_text("HEL", 300, 200, 0x000000);
	danaos_deinit();
	return 0;
}
