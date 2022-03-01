#include <system.h>

static int broadcastType = 0;
static int broadcastMessage = NULL;
static Broadcast broadcast;

void send_broadcast(int type, char* message) {
	broadcastType = type;
	broadcastMessage = message;
}

Broadcast* receive_broadcast() {
	if (broadcastType == 0 && broadcastMessage == NULL) {
		return NULL;
	}
	broadcast.type = broadcastType;
	broadcast.message = broadcastMessage;
	broadcastType = 0;
	broadcastMessage = NULL;
	return &broadcast;
}
