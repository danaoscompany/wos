#include <system.h>

int current_utc = 0;
int current_second;
int current_minute;
int current_hour;
int current_day;
int current_weekday;
int current_month;
int current_year;
int current_century;

void cmos_write(char address, char value) {
	outportb(0x70, address);
	sleep(10);
	outportb(0x71, value);
	sleep(10);
}

char cmos_read(char address) {
	outportb(0x70, address);
	sleep(10);
	char value = inportb(0x71);
	sleep(10);
}

int cmos_get_second() {
	current_second = (int)cmos_read(0x00);
	return current_second;
}

int cmos_get_minute() {
	current_minute = (int)cmos_read(0x02);
	return current_minute;
}

int cmos_get_hour() {
	current_hour = (int)cmos_read(0x04);
	current_hour += get_utc();
	return current_hour;
}

int cmos_get_day() {
	current_day = (int)cmos_read(0x07);
	current_day += current_hour/24;
	return current_day;
}

int cmos_get_weekday() {
	current_weekday = (int)cmos_read(0x06);
	current_weekday += current_hour/24;
	return current_weekday;
}

int cmos_get_month() {
	current_month = (int)cmos_read(0x08);
	current_month += current_day/30;
	return current_month;
}

int cmos_get_year() {
	current_year = (int)cmos_read(0x09);
	current_year += current_month/12;
	return current_year;
}

int cmos_get_century() {
	current_century = (int)cmos_read(0x32);
	return current_century;
}

void set_utc(int utc) {
	current_utc = utc;
}

int get_utc() {
	return current_utc;
}
