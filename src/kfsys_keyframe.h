#ifndef KFSYS_KEYFRAME_H
#define KFSYS_KEYFRAME_H

#include "kfsys_sound.h"

class Keyframe {
	public:
		int start;
		int flags;
		Sound *snd;
		float pos[3];
		float note_ratio;
		
		Keyframe() { snd = NULL; }
};

#endif
