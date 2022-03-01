#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#define TYPE_TIMER_OCCURED 0

typedef struct {
	int type;
	int id;
} Notification;

void init_notifications();
void notify(int type, int id);
Notification** getNotifications();
int getTotalNotification();

#endif
