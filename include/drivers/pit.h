#ifndef PIT_H
#define PIT_H

typedef struct {
	long realtime; /* Time from start of the execution of a program until "now" */
	long cputime; /* User-Code time, how long does it take to run a user code */
	long systemtime; /* Kernel-Code time, how long does it take to run a kernel code */
	int pid;
} TimeCapture;

typedef struct {
	long long tms_utime;
	long long tms_stime;
	long long tms_cutime;
	long long tms_cstime;
} Times;

typedef struct {
	int stopTime;
	int ticker;
	int timerId;
} Schedule;

void init_pit(int millis_per_second);
void sleep(long wait_time);
void times(int pid, Times* time_info);
void schedule(int time, int timerId);
bool isTimerOccured(int id);

#endif
