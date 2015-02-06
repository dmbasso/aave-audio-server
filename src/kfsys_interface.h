#ifndef KFSYS_INTERFACE_H
#define KFSYS_INTERFACE_H

#include <map>

#include "aave_interface.h"
#include "kfsys_source.h"

#define SAMPLERATE 44100
#define BUFFLEN 2048
#define MAX_SOURCES 5

using namespace std;

enum class kfsys_cmds {
	listener = 0,
	source = 1,
	geometry = 2,
	reverb = 3,
	input_params = 4,
	output = 5
};

enum class output_cmds {
	iterate = 0,
	set_frame = 1,
	write_frames = 2
};

enum class input_params_cmds {
    mode = 0,
    hrtf = 1,
    reflections = 2,
    frame_rate = 3,
    audio_engine = 4
};

enum class reverb_cmds {
    add = 0,
    area = 1,
    volume = 2,
    rt60 = 3,
    reverb_active = 4,
    reverb_mix = 5
};

enum class geometry_cmds {
    add = 0,
    set_geometry = 1,
    set_posrot = 2,
    set_material = 3
};

class KFSystem {

	public:
	    static Libaave *libaave;
    	map<short, Source *> sources;
		short audio_engine; //0 = direct, 1 = aave
		int write_frames;
    	uint64_t global_position;
    	int32_t delay;
    	
    	KFSystem();
    	void render(short *buff, int bufflen);
    	void start_keyframes(int delay);
		void set_audio_engine(short ae);
		struct aave* get_aave_engine();
		struct aave_surface* get_aave_surfaces();
		short set_aave_hrtf(short hrtf);
		void set_listener_position(float x, float y, float z);
		void set_listener_orientation(float roll, float pitch, float yaw);
		float* get_listener_position();
		void set_reflection_order(unsigned n);
		unsigned get_reflection_order();
		void update_aave_geometry();
		void init_aave_reverb();
		void set_aave_reverb_rt60(unsigned short rt60);
		void set_aave_reverb_area(unsigned short area);
		void set_aave_reverb_volume(unsigned short volume);
		void enable_disable_aave_reverb();
		void set_aave_gain(float gain);
		void increase_aave_gain();
		void decrease_aave_gain();
		void add_source(int id);
		struct aave_source* get_aave_source(short id);
    	int done();
    	
    	void handle_datagram(char *, int);
    	short handle_reverb_cmds(char *, int);
    	short handle_input_params_cmds(char *, int);
    	short handle_geometry_cmds(char *, int);
    	short cmds_listener_set_position(char *, int);
    	short cmds_output_iterate(char *, int);
    	short cmds_output_set_frame(char *, int);
		short cmds_output_write_frames(char *, int);
    	short cmds_add_source(char *, int);
    	short remove_source(char *, int);
    	short clear_sources(char *, int);
};

#endif
