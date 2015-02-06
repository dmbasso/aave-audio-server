#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "util.h"

using namespace std;

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    char     datachunk_id[4];
    uint32_t datachunk_size;
}WavHeader;

void init_output_wavfile(ofstream *ofs, int data_size) {

	WavHeader header;
	
	strncpy(header.chunk_id,"RIFF",4);
	header.datachunk_size = data_size;
	header.chunk_size = header.datachunk_size + 36;
	strncpy(header.format,"WAVE",4);
	header.num_channels = 2;
	strncpy(header.fmtchunk_id,"fmt ",4);
	header.fmtchunk_size = 16;
	header.audio_format = 1;
	header.sample_rate = 44100;
	header.byte_rate = (header.sample_rate * header.bps * header.num_channels) / 8;
	header.block_align = header.num_channels * (header.bps >> 3);
	header.bps = 16;
	strncpy(header.datachunk_id,"data",4);
	
	ofs->write((char *) &header, sizeof(WavHeader));
}

void buffer_stereo_to_mono(short *buff, int len) {
	for (int i=0; i<len; i++)
		if (!(i%2)) buff[i/2] = buff[i];
}

