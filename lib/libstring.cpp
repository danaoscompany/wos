#include <libsystem.h>

int strlen(char* text) {
	int total = 0;
	while (text[total] != 0) {
		total++;
	}
	return total;
}
