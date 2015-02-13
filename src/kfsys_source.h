#ifndef KFSYS_SOURCE_H
#define KFSYS_SOURCE_H

#include <vector>


#ifdef WITH_AAVE
    #include "aave_interface.h"
#endif
#include "kfsys_sound.h"
#include "kfsys_keyframe.h"

enum class source_cmds {
	add = 0,
	set_pos = 1,
	put_audio = 2,
	set_sound_file = 3,
	start_sound = 4,
	clear_keyframes = 5,
	add_keyframes = 6
}; 

class KFSystem;

class Source {

	public:
        #ifdef WITH_AAVE
    		struct aave_source *aave_source;
		#endif
		Sound *sound; //map
		vector<Keyframe> keyframes;
		int loop;
    	int keyframe_active;
    	uint64_t keyframe_start_position;
    	int current_keyframe;
    	int sample_position;
    	
    	Source();
    	void clear_keyframes();
    	void set_sound(Sound *snd);
    	int  done();
		void start_keyframes(KFSystem *sys, int delay);
		void render(uint64_t global_position, short *buff, int bufflen);
		
		#ifdef WITH_AAVE
            void init_aave(Libaave *libaave);
        #endif
		void set_position(float, float, float);
		void get_position(float*);

    	short handle_datagram(char *, int);
    	short handle_add_keyframe(char *, int);
    	void cmds_source(char *, int);
};

#endif
