#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "kfsys_source.h"
#include "kfsys_interface.h"

inline float unpack_fl (char* buf) {
    int itemp = ntohl(*(long*) buf);
    return *(float*) &itemp;
}

Source::Source() {
    #ifdef WITH_AAVE
    	aave_source = NULL;
	#endif
    sound = NULL;
    keyframe_active = 0;
    sample_position = -1;
    note_ratio = 1;
    current_keyframe = -1;
    loop = 0;
}

void Source::clear_keyframes() {
    printf("clearing keyframes\n");
    keyframes.clear();
}

void Source::set_sound(Sound *snd) {
	sound = snd;
}

int Source::done() {
    return current_keyframe >= keyframes.size() && sample_position == -1;
}

void Source::start_keyframes(KFSystem *sys, int delay) {
    keyframe_start_position = sys->global_position + delay;
    keyframe_active = 1;
    current_keyframe = -1;
    sample_position = -1;
}

void Source::render(uint64_t global_position, short *buff, int frames) {
    
    if (!keyframe_active) {
        // could still play other stuff
        return;
    }
    
    int i;
    int position = global_position - keyframe_start_position;
    int in_buff_pos = 0;

    while (in_buff_pos < frames) {
        /*printf("%p: %i, %i <= %i < %i?\n",
               src, src->current_keyframe, position,
               src->keyframes[src->current_keyframe].start,
               position + bufflen);*/
        int limit = frames;
        // first we must check if we have to forward keyframes
        while (current_keyframe < 0 || (current_keyframe < keyframes.size() &&
               position > keyframes[current_keyframe].start)) {
            current_keyframe += 1;
        }
        // check if the current keyframe will be interrupted by the next one
        if (current_keyframe < keyframes.size()
        && position <= keyframes[current_keyframe].start
        && position + frames > keyframes[current_keyframe].start) {
            limit = keyframes[current_keyframe].start - position;
        }
        // now we render the current sound (if any) until the limit
        if (sound && sound->samples && sample_position >= 0) {
            for (i=in_buff_pos; i<limit; i++) {
                short samples[2];
                sound->get_stereo_frame(samples, sample_position);
                buff[i * 2] += samples[0];
                buff[i * 2 + 1] += samples[1];
                sample_position += note_ratio;
                if (sample_position >= sound->length) {
                    sample_position = loop ? 0 : -1;
                }
            }
        }
        if (current_keyframe < keyframes.size()
        && position <= keyframes[current_keyframe].start
        && position + frames > keyframes[current_keyframe].start) {
            printf("%p: %i, %i <= %i < %i :: gpos %lu\n",
                   (void *) this, current_keyframe, position,
                   keyframes[current_keyframe].start,
                   position + frames, global_position);
            // now we could change position, sound (instrument), frequency, whatever
            if (keyframes[current_keyframe].flags & 2) {
				set_position(
                    keyframes[current_keyframe].pos[0],
                    keyframes[current_keyframe].pos[1],
                    keyframes[current_keyframe].pos[2]);
            }
            if (keyframes[current_keyframe].flags & 8) {
                note_ratio = keyframes[current_keyframe].note_ratio;
            } else {
                note_ratio = 1;
            }
            if (1) { // src->keyframes[src->current_keyframe].flags & PLAY
                sample_position = 0;
            }
            in_buff_pos = keyframes[current_keyframe].start - position;
            current_keyframe += 1;
        } else {
            // we're done for this cycle
            return;
        }
    }
}

#ifdef WITH_AAVE
void Source::init_aave(Libaave *libaave) {

	aave_source = (struct aave_source *) malloc(sizeof (struct aave_source));
	aave_init_source(libaave->aave, aave_source);
	aave_add_source(libaave->aave, aave_source);
}
#endif

void Source::set_position(float x, float y, float z) {
    #ifdef WITH_AAVE
    	aave_set_source_position(this->aave_source, x, y, z);
	#endif
}

void Source::get_position(float* pos) {
    #ifdef WITH_AAVE
	pos[0] = this->aave_source->position[0];
	pos[1] = this->aave_source->position[1];
	pos[2] = this->aave_source->position[2];
	#endif
}

short Source::handle_datagram(char *recv_buf, int recv_len) {

	printf("source cmd = %d\n", recv_buf[1]);
	
	switch (static_cast<source_cmds>(recv_buf[1])) {
		case source_cmds::set_pos:
		{
		    float x = unpack_fl(recv_buf + 3);
		    float y = unpack_fl(recv_buf + 7);
		    float z = unpack_fl(recv_buf + 11);
		    set_position(x, y, z);
		    return 3 + 3 * 4;	
	    }

		case source_cmds::put_audio:
		{
		    break;
		}   
		case source_cmds::set_sound_file:
		{
		    int len = (unsigned char) recv_buf[3];
		    string name(&recv_buf[4], len);
		    sound = Sound::get_sound(name);
		    printf("sound %p %i\n", sound, sound ? sound->length : -1);
		    return 3 + 1 + len;
	    }
		case source_cmds::start_sound:
        {
            sample_position = 0;
            return 3;
	    }
		case source_cmds::clear_keyframes:
		{
		    clear_keyframes();
		    return 3;
	    }
		case source_cmds::add_keyframes:
		{
            return handle_add_keyframe(recv_buf, recv_len);
	    }
	}
}

short Source::handle_add_keyframe(char *recv_buf, int recv_len) {
    printf("Source::handle_add_keyframe\n");

	Keyframe kf;
	float kf_pos[3];
	short retv = 4 + 4; // cmds + time
	
	kf.flags = recv_buf[3];
	// convert time from milliseconds to audio frames at 44.1KHz
	kf.start = ntohl(*(long*)(recv_buf + 4)) * 44100 / 1000;
    recv_buf += 8;

	if (kf.flags & 2) {
		kf.pos[0] = unpack_fl(recv_buf);
		kf.pos[1] = unpack_fl(recv_buf + 4);
		kf.pos[2] = unpack_fl(recv_buf + 8);
		retv += 3 * 4;
        recv_buf += 12;
	}
    if (kf.flags & 8) {
	    kf.note_ratio = unpack_fl(recv_buf);
	    retv += 4;
	    recv_buf += 4;
    }
	//if (kf.flags & anything_else) will fuck up;
	keyframes.push_back(kf);

	printf("KF: %d, flags = %d, pos = %f,%f,%f\n", kf.start, kf.flags, kf.pos[0], kf.pos[1], kf.pos[2]);
	return retv;
}
