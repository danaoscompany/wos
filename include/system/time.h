#ifndef TIME_H
#define TIME_H

#define CMOS 0

int get_second();
int get_minute();
int get_hour();
int get_day();
int get_weekday();
int get_month();
int get_year();
int get_century();
void set_time_server(int _server);

#endif
