#include <system.h>

AudioDriver* audioDriver;
static Device* audioDevice;
bool audioDriverInitialized = false;
static int deviceId;

void init_audio_driver() {
	audioDriver = (AudioDriver*)malloc(sizeof(AudioDriver));
	audioDevice = get_device(0x4, 0x3);
	if (audioDevice == NULL) {
		printf("There is no such device");
		return;
	}
	/* Search for device name, by using pci.ids file */
	char* deviceName = get_device_name(audioDevice);
	if (search_string(deviceName, "High Definition Audio Controller", 0, 0)) {
		audioDriver->init = init_intel_hda;
	}
	if (audioDriver->init != 0) {
		audioDriver->init();
	}
}

void audio_driver_set_init_method(void (*init)()) {
	audioDriver->init = init;
}

void audio_driver_set_device_id(int _deviceId) {
	deviceId = _deviceId;
}

void enable_audio_driver() {
	int status = audioDriver->init();
	if (status == DRV_INIT_OK) {
		//Driver initialized successfully
		audioDriverInitialized = true;
	}
}

Device* get_audio_device() {
	return audioDevice;
}

AudioDriver* get_audio_driver() {
	return audioDriver;
}
