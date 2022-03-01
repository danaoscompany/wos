#include <system.h>

static int server;

void set_time_server(int _server) {
	server = _server;
}

int get_second() {
	if (server == CMOS) {
		return cmos_get_second();
	}
}

int get_minute() {
	if (server == CMOS) {
		return cmos_get_minute();
	}
}

int get_hour() {
	if (server == CMOS) {
		return cmos_get_hour();
	}
}

int get_day() {
	if (server == CMOS) {
		return cmos_get_weekday();
	}
}

int get_weekday() {
	if (server == CMOS) {
		return cmos_get_weekday();
	}
}

int get_month() {
	if (server == CMOS) {
		return cmos_get_month();
	}
}

int get_year() {
	if (server == CMOS) {
		return cmos_get_year();
	}
}

int get_century() {
	if (server == CMOS) {
		return cmos_get_century();
	}
}
