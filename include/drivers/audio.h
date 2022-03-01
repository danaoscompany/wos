#ifndef AUDIO_H
#define AUDIO_H

typedef struct {
	int (*init)();
} AudioDriver;

void init_audio_driver();
void audio_driver_set_init_method(void (*init)());
void audio_driver_set_device_id(int _deviceId);
extern AudioDriver* audioDriver;
void enable_audio_driver();
Device* get_audio_device();
AudioDriver* get_audio_driver();

#endif
