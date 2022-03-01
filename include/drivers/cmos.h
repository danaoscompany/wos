#ifndef CMOS_H
#define CMOS_H

void cmos_write(char address, char value);
char cmos_read(char address);
int cmos_get_second();
int cmos_get_minute();
int cmos_get_hour();
int cmos_get_day();
int cmos_get_weekday();
int cmos_get_month();
int cmos_get_year();
int cmos_get_century();
void set_utc(int utc);
int get_utc();

#endif
