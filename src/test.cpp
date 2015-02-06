#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "kfsys_source.h"
#include "kfsys_keyframe.h"
#include "kfsys_interface.h"
#include "alsa_interface.h"
#include "test.h"
#include "util.h"

KFSystem sys;
int aave_delay;

extern "C" {

struct aave* get_aave_engine() {
	return sys.get_aave_engine();
}

struct aave_source* get_aave_source(short id) {
	return sys.get_aave_source(id);
}

aave_surface* get_aave_surfaces() {
	return sys.get_aave_surfaces();
}

void set_audio_engine(short ae) {
	sys.set_audio_engine(ae);
	printf("Using audio engine %d\n",ae);
}

short set_hrtf (short hrtf) {
	return sys.set_aave_hrtf(hrtf);
	printf("Using hrtf set %d\n",hrtf);
}

void set_listener_position(float x, float y, float z) {
	std::cout << "setting listener position: " << x << y << z << endl;
	sys.set_listener_position(x,y,z);
}

float* get_listener_position() {
	return sys.get_listener_position();
}

void set_listener_orientation(float roll, float pitch, float yaw) {
	sys.set_listener_orientation(roll, pitch, yaw);
}

void set_geometry(const char* obj) {
	aave_read_obj(sys.get_aave_engine(), obj);
}

void set_reflection_order(unsigned n) {
	sys.set_reflection_order(n);
}

unsigned get_reflection_order() {
	return sys.get_reflection_order();
}

void add_source(int id) {
	sys.add_source(id);
}

void set_source_sound(int id, const char* name) {

    if (sys.sources.find(id) != sys.sources.end()) {
    	sys.sources.at(id)->sound = Sound::get_sound(name);
		printf("loading sound %p %i\n", sys.sources.at(id)->sound, sys.sources.at(id)->sound ? sys.sources.at(id)->sound->length : -1);
    }
    else printf("source %d not initialized...\n", id);
}

void set_source_position(int id, float x, float y, float z) {

    if (sys.sources.find(id) != sys.sources.end() && sys.audio_engine == 1) {
		printf("setting source %d position: %.2f, %.2f, %.2f\n", id, x, y, z);
    	sys.sources.at(id)->set_position(x, y, z);
    }
}

float* get_source_position(int id) {

	if (sys.sources.find(id) != sys.sources.end() && sys.audio_engine == 1)
		return sys.sources.at(id)->get_position();
    else
		return NULL;
}

void source_start_sound(int id) {

    if (sys.sources.find(id) != sys.sources.end())
		sys.sources.at(id)->sample_position = 0;
}

void source_add_keyframe(int id, int start,	int flags, float posx, float posy, float posz) {

    if (sys.sources.find(id) != sys.sources.end()) {
    	Keyframe kf;
    	kf.flags = flags;
    	kf.start = start;
		if (kf.flags & 2 && sys.audio_engine == 1) {
			kf.pos[0] = posx;
			kf.pos[1] = posy;
			kf.pos[2] = posz;
		}
		sys.sources.at(id)->keyframes.push_back(kf);
	}
}

void source_clear_keyframes(int id) {

	if (sys.sources.find(id) != sys.sources.end())
		sys.sources.at(id)->clear_keyframes();
}

void start_keyframes(int delay) {

	printf("starting with delay %i\n", delay);
    for (unsigned i=0; i < sys.sources.size(); i++) {
        sys.sources[i]->start_keyframes(&sys, delay);
    }
}

void source_convert_stereo_to_mono(int source_id) {
	if (sys.sources.find(source_id) != sys.sources.end())
		sys.sources.at(source_id)->sound->convert_stereo_to_mono();
}

void source_write_sound_file(int source_id) {
	if (sys.sources.find(source_id) != sys.sources.end())
		sys.sources.at(source_id)->sound->write_sound_file();
}

void aave_update_engine() {
	sys.update_aave_geometry();
}

void init_reverb() {
	sys.init_aave_reverb();
}

void set_reverb_rt60(unsigned short rt60) {
	sys.set_aave_reverb_rt60(rt60);
}

void set_reverb_area(unsigned short area) {
	sys.set_aave_reverb_area(area);
}

void set_reverb_volume(unsigned short volume) {
	sys.set_aave_reverb_volume(volume);
	aave_reverb_print_parameters(sys.get_aave_engine(), sys.get_aave_engine()->reverb);
}

void enable_disable_reverb() {
	sys.enable_disable_aave_reverb();
}

void set_gain(float gain) {
	sys.set_aave_gain(gain);
}

void increase_gain() {
	sys.increase_aave_gain();
}

void decrease_gain() {
	sys.decrease_aave_gain();
}


void render_frames_tofile(int nframes) {

	int data_size = nframes * BUFFLEN * 2 * 2; //16 bit stereo frames
    short buff[BUFFLEN * 2];
    memset(buff, 0, BUFFLEN * 2);
    
    std::ofstream ofs;
    ofs.open(("../sounds/output/output.wav"),std::ofstream::out);
    init_output_wavfile(&ofs, data_size);

    while (nframes--) {
		sys.render(buff, BUFFLEN);		
		ofs.write((char *) &buff,2*2*BUFFLEN);
		// recv iterate
    }
    ofs.close();
}

int render_frames_todriver(int nframes) {

    short buff[BUFFLEN * 2];

    Alsa alsa;
    alsa.setup(44100, 2, 8192);

    while (nframes--) {
		sys.render(buff, BUFFLEN);
		alsa.write(buff, BUFFLEN);
		// recv iterate
    }
    return 1;
}

} // extern "C"
