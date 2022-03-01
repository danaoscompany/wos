#ifndef AC97_H
#define AC97_H

typedef struct {
	uint32_t phys;
	uint16_t samples;
	uint16_t misc;
} BufferDescriptorList;

void init_ac97();
void ac97_play(uint8_t* buffer, uint32_t length, uint16_t sampleRate, uint8_t channelCount);
void ac97_pause();
void ac97_resume();
void ac97_handler(Register* r);
void testAC97();

#endif