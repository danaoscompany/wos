#include <math.h>
#include <vorbis/codec.h>

ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
int convsize=4096;

uint8_t* srcData = 0;
uint32_t nextPtr = 0;
uint32_t nextDstPtr = 0;
uint8_t dstBuffer[100*1024*1024];

int read_data(uint8_t* buffer, uint32_t length) {
  memcpy(buffer, srcData+nextPtr, length);
  nextPtr += length;
  return length;
}

void write_data(uint8_t* buffer, uint32_t length) {
	memcpy(dstBuffer+nextDstPtr, buffer, length);
	nextDstPtr += length;
}

void vorbis_test() {
	ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
  ogg_packet       op; /* one raw packet of data for decode */

  vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                          settings */
  vorbis_comment   vc; /* struct that stores all the bitstream user comments */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  char *buffer;
  int  bytes;
  ogg_sync_init(&oy); /* Now we can read pages */

  
}
