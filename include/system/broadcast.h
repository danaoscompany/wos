#ifndef BROADCAST_H
#define BROADCAST_H

/* Broadcast types */
#define SCRIPT_MESSAGE 1

typedef struct {
	int type;
	char* message;
} Broadcast;

void send_broadcast(int type, char* message);
Broadcast* receive_broadcast();

#endif
