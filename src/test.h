#ifndef TEST_H
#define TEST_H

#include <map>

#include "kfsys_interface.h"
#include "aave_interface.h"

extern "C" {

// AAVE only commands
struct aave* get_aave_engine();
struct aave_source* get_aave_source(short id);
short set_hrtf (short hrtf);
void set_source_position(int id, float x, float y, float z);
void get_source_position(int id, float*);
void set_listener_position(float x, float y, float z);
float* get_listener_position();
void set_listener_orientation(float x, float y, float z);
void set_geometry(const char* obj);
void set_reflection_order(unsigned n);
unsigned get_reflection_order();
void init_reverb();
void set_reverb_rt60(unsigned short);
void set_reverb_area(unsigned short);
void set_reverb_volume(unsigned short);
void aave_update_engine();
void set_gain(float);
void increase_gain();
void decrease_gain();
void enable_disable_reverb();

aave_surface* get_aave_surfaces();

// AAVE + DIRECT common commands
void set_audio_engine(short ae);
void add_source(int id);
void set_source_sound(int id, const char* name);
void source_start_sound(int id);
void source_add_keyframe(int id, int start, int flags, float posx, float posy, float posz);
void source_clear_keyframes(int id);
void start_keyframes(int delay);
void render_frames_tofile(int nframes);
int render_frames_todriver(int nframes);

//utils
void source_convert_stereo_to_mono(int source_id);
void source_write_sound_file(int source_id);
}

#endif
