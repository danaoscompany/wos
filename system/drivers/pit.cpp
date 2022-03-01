#include <system.h>

static bool waiting = false;
static int time = 0;
TimeCapture** time_captures;
int total_time_captures;
int max_time_captures = 10;
Schedule** schedules;
int totalSchedule = 0;
int maxSchedules = 10;

void pit_handler(Register* r) {
	for (int i=0; i<totalSchedule; i++) {
		Schedule* schedule = schedules[i];
		if (schedule != 0) {
			if (schedule->ticker < schedule->stopTime) {
				schedule->ticker++;
			} else {
				notify(TYPE_TIMER_OCCURED, schedule->timerId);
				free(schedule);
				schedules[i] = 0;
			}
		}
	}
	if (waiting) {
		time++;
	}
	for (int i=0; i<total_time_captures; i++) {
		TimeCapture* tm = time_captures[i];
		if (tm->realtime < MAX_LONG) {
			tm->realtime++;
		} else {
			tm->realtime = 0;
		}
	}
}

void sleep(long wait_time) {
	waiting = true;
	while (time < wait_time) {
		wait();
	}
	time = 0;
}

static void add_schedule(int time, int timerId) {
	if (totalSchedule >= maxSchedules) {
		maxSchedules *= 2;
		Schedule** tmp = (Schedule**)malloc(sizeof(Schedule*)*maxSchedules);
		for (int i=0; i<totalSchedule; i++) {
			tmp[i] = schedules[i];
		}
		free(schedules);
		schedules = tmp;
	}
	Schedule* schedule = (Schedule*)malloc(sizeof(Schedule));
	schedule->stopTime = time;
	schedule->ticker = 0;
	schedule->timerId = timerId;
	schedules[totalSchedule] = schedule;
	totalSchedule++;
}

void schedule(int time, int timerId) {
	add_schedule(time, timerId);
}

TimeCapture* find_time_capture(int pid) {
	for (int i=0; i<total_time_captures; i++) {
		if (time_captures[i]->pid == pid) {
			return time_captures[i];
		}
	}
	return NULL;
}

void times(int pid, Times* time_info) {
	TimeCapture* tc = find_time_capture(pid);
	if (tc != NULL) {
		time_info->tms_utime = tc->cputime;
		time_info->tms_stime = tc->systemtime;
		time_info->tms_cutime = tc->cputime;
		time_info->tms_cstime = tc->systemtime;
	}
}

void add_time_capture(TimeCapture* tc) {
	if (total_time_captures >= max_time_captures) {
		max_time_captures *= 2;
		TimeCapture** tmp = (TimeCapture**)malloc(sizeof(TimeCapture*)*max_time_captures);
		for (int i=0; i<total_time_captures; i++) {
			tmp[i] = time_captures[i];
		}
		free(time_captures);
		time_captures = tmp;
	}
	time_captures[total_time_captures] = tc;
	total_time_captures++;
}

void capture_time(int pid) {
	TimeCapture* tc = (TimeCapture*)malloc(sizeof(TimeCapture));
	tc->pid = pid;
	add_time_capture(tc);
}

void init_pit(int millis_per_second) {
	int a = 1193180/millis_per_second;
	outportb(0x43, 0x36);
	outportb(0x40, a&0xFF);
	outportb(0x40, a>>8&0xFF);
	time_captures = (TimeCapture**)malloc(sizeof(TimeCapture*)*max_time_captures);
	schedules = (Schedule**)malloc(sizeof(Schedule*)*maxSchedules);
	irq_set_handler(0, pit_handler);
}

bool isTimerOccured(int id) {
	Notification** notifications = getNotifications();
	for (int i=0; i<getTotalNotification(); i++) {
		Notification* n = notifications[i];
		if (n->type == TYPE_TIMER_OCCURED && n->id == id) {
			return true;
		}
	}
	return false;
}
