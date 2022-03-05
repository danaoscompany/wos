#include <wos/system.h>
#include <stdarg.h>
#include <stdint.h>

void create_window(Window* window) {
	sys_call(0x14, (uint32_t)window, 0, 0);
}
