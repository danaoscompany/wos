#include <system.h>

Notification** notifications;
int totalNotification = 0;
int maxNotifications = 10;

static void add_notification(int type, int id) {
	if (totalNotification >= maxNotifications) {
		maxNotifications *= 2;
		Notification** tmp = (Notification**)malloc(sizeof(Notification)*maxNotifications);
		for (int i=0; i<totalNotification; i++) {
			tmp[i] = notifications[i];
		}
		free(notifications);
		notifications = tmp;
	}
	Notification* notification = (Notification*)malloc(sizeof(Notification));
	notification->type = type;
	notification->id = id;
	notifications[totalNotification] = notification;
	totalNotification++;
}

void notify(int type, int id) {
	add_notification(type, id);
}

void init_notifications() {
	notifications = (Notification**)malloc(sizeof(Notification*)*maxNotifications);
}

Notification** getNotifications() {
	return notifications;
}

int getTotalNotification() {
	return totalNotification;
}
