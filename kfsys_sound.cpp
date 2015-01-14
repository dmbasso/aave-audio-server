#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

#include <fstream>

#include "kfsys_sound.h"

Sound::Sound(string &file_name) {

    int fd;
    #define errx(x,msg) {samples=NULL; length=0; printf(msg); return;}
    if ((fd = open(file_name.c_str(), O_RDONLY)) < 1)
        errx(1, "Error opening file\n");
    if (read(fd, &header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "File broken: header");
    if (strncmp(header.chunk_id, "RIFF", 4) ||
        strncmp(header.format, "WAVE", 4))
        errx(1, "Not a wav file");
    if (header.audio_format != 1)
        errx(1, "Only PCM encoding supported");
    channels = header.num_channels;
    int bytes_per_sample = header.bps / 8;
    int sample_count = header.datachunk_size / bytes_per_sample;
    // here we should convert any other bps to 16... for now lets just
    // assume it is 16
    samples = (int16_t*)malloc(bytes_per_sample * sample_count);
    if (!samples)
        errx(1, "Error allocating memory");
    if (read(fd, samples, header.datachunk_size) < header.datachunk_size)
        errx(1, "File broken: samples");
    close(fd);
	length = sample_count / channels;
}

map<string, Sound*> Sound::sounds;

Sound* Sound::get_sound(string name)
{
    if (!sounds.count(name)) {
        Sound *snd = new Sound(name);
        sounds[name] = snd;
    }
    return sounds[name];
}

void Sound::convert_stereo_to_mono() {

	int i,sample_count;
	short *new_samples;

	if (channels == 1) {
		printf("Sound is already mono...\n");
		return;
	}

	new_samples = (short*)malloc(sizeof(short)*header.datachunk_size/2);

	for (i = 0; i < header.datachunk_size; i += 2)
		new_samples[i/2] = (samples[i] + samples[i+1]) / 2;

	free(samples);
	samples = new_samples;
	channels = 1;

	sample_count = (header.datachunk_size/2) / (header.bps / 8);
	length = sample_count / channels;

	header.datachunk_size = header.datachunk_size/2;
	header.chunk_size = header.datachunk_size + 36;
	header.num_channels = 1;
	header.byte_rate = (header.sample_rate * header.bps * header.num_channels) / 8;
	header.block_align = header.num_channels * (header.bps >> 3);
}

void Sound::write_sound_file() {

	std::ofstream ofs;
    ofs.open(("../sounds/output/mono_snd.wav"),std::ofstream::out);
	ofs.write((char *) &header, sizeof(WavHeader));
	ofs.write((char *) samples, header.datachunk_size);
}
