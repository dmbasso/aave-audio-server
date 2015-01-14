#ifndef AAVE_INTERFACE_H
#define AAVE_INTERFACE_H

extern "C" {
#include "../libaave/aave.h"
}

#define MAX_VERTICES 1024

class Libaave {

	public:
		struct aave *aave;
		struct aave_reverb *reverb;
		//data structure to load geometries.
		
		Libaave();
		void set_listener_position(float, float, float);
		void set_listener_orientation(float, float, float);
		short set_geometry(char *recv_buf, int recv_len);
		short set_geometry_material(char *recv_buf, int recv_len);
		void update_geometry();
		void put_audio(struct aave_source*, short *, int);
		void get_binaural_audio(short *, int);
		void set_gain(float);
		void init_reverb();
		void set_reverb_rt60(unsigned short);
		void set_reverb_area(unsigned short);
		void set_reverb_volume(unsigned short);
		
		
};

#endif
