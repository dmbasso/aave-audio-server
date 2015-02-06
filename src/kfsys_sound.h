#ifndef KFSYS_SOUND_H
#define KFSYS_SOUND_H

#include <map>
#include <string>
#include <string.h>
#include <stdint.h>

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

class Sound {
    private:
	    static map<string, Sound*> sounds;

	public:
		WavHeader header;
		short *samples;
		int length;
		int channels;

		Sound(string &name);
        static Sound* get_sound(string name);
        void convert_stereo_to_mono();
        void write_sound_file();
};

#endif
