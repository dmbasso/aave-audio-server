#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>

#include "kfsys_source.h"
#include "kfsys_keyframe.h"
#include "kfsys_interface.h"
#include "test.h"
#include "util.h"

KFSystem sys;
int aave_delay;

extern "C" {

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

	sys.libaave->set_listener_position(x,y,z);
}

void set_listener_orientation(float x, float y, float z) {

	sys.libaave->set_listener_orientation(x,y,z);
}

void set_geometry(const char* obj) {

	aave_read_obj(sys.libaave->aave, obj);
}

void set_reflection_order(int n) {

	sys.libaave->aave->reflections = n;
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
    
    if (sys.sources.find(id) != sys.sources.end() && sys.audio_engine == 1)
    	sys.sources.at(id)->set_position(x, y, z);
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
    int i;
    for (i=0; i < sys.sources.size(); i++) {
        sys.sources[i]->start_keyframes(&sys, delay);
    }
}

int render_nframes(int nframes) {

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
}

void set_gain(float gain) {
	sys.libaave->set_gain(gain);
}

} // extern "C"
