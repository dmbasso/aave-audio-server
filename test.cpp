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

	if (sys.libaave->aave != NULL)
		return sys.libaave->aave;
	return NULL;
}

struct aave_source* get_aave_source(short id) {

	if (sys.sources.find(id) != sys.sources.end())
		return sys.sources.at(id)->aave_source;
	return NULL;
}

aave_surface* get_aave_surfaces() {
	return sys.libaave->aave->surfaces;
}

void set_audio_engine(short ae) {

	sys.set_audio_engine(ae);
	printf("Using audio engine %d\n",ae);
}

void set_hrtf (short hrtf) {

	if (hrtf == 1) {
		aave_hrtf_mit(sys.libaave->aave);
		aave_delay = -333;
	}
	if (hrtf == 2) {
		aave_hrtf_cipic(sys.libaave->aave);
		aave_delay = -598;
	}
	if (hrtf == 3) {
		aave_hrtf_listen(sys.libaave->aave);
		aave_delay = -1124;
	}
	if (hrtf == 4) {
		aave_hrtf_tub(sys.libaave->aave);
		aave_delay = -2169;
	}
}

void set_listener_position(float x, float y, float z) {
	std::cout << "setting listener position: " << x << y << z << endl;
	sys.libaave->set_listener_position(x,y,z);
}

float* get_listener_position() {
	return sys.libaave->get_listener_position();
}

void set_listener_orientation(float x, float y, float z) {

	sys.libaave->set_listener_orientation(x,y,z);
}

void set_geometry(const char* obj) {

	aave_read_obj(sys.libaave->aave, obj);
}

void set_reflection_order(unsigned n) {

	sys.libaave->aave->reflections = n;
}

unsigned get_reflection_order() {

	return sys.libaave->aave->reflections;
}

void add_source(int id) {

    auto source = new Source;

    if (sys.sources.find(id) == sys.sources.end()) {
    	printf("adding source: %d\n", id);
		sys.sources.insert(make_pair(id, source));
		if (sys.audio_engine == 1)
			source->init_aave(sys.libaave);
	}
	else printf("Source %d already allocated...\n", id);
}

void set_source_sound(int id, const char* name) {

    if (sys.sources.find(id) != sys.sources.end()) {
    	sys.sources.at(id)->sound = Sound::get_sound(name);
		printf("loading sound %p %i\n", sys.sources.at(id)->sound, sys.sources.at(id)->sound ? sys.sources.at(id)->sound->length : -1);
    }
    else printf("Source %d not initialized...\n", id);
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
    alsa.setup_default();

    while (nframes--) {
		sys.render(buff, BUFFLEN);
		ofs.write((char *) &buff, 2*2*BUFFLEN);
		alsa.write(buff, BUFFLEN);
		// recv iterate
    }
    return 1;
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
	sys.libaave->update_geometry(); /* updates geometries + sources */
}

void init_reverb() {
	sys.libaave->init_reverb();
}

void set_reverb_rt60(unsigned short rt60) {
	sys.libaave->set_reverb_rt60(rt60);
}

void set_reverb_area(unsigned short area) {
	sys.libaave->set_reverb_area(area);
}

void set_reverb_volume(unsigned short volume) {
	sys.libaave->set_reverb_volume(volume);
	aave_reverb_print_parameters(sys.libaave->aave, sys.libaave->aave->reverb);
}

void enable_disable_reverb() {
	sys.libaave->enable_disable_reverb();
}

void set_gain(float gain) {
	sys.libaave->set_gain(gain);
}

void increase_gain() {
	sys.libaave->increase_gain();
}

void decrease_gain() {
	sys.libaave->decrease_gain();
}

} // extern "C"
