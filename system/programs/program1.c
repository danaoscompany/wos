#include <fcntl.h>

int main() {
	sys_call(0x11, 0, 0, 0);
	return 0;
}
