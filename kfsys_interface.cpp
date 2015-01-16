#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> //abort()
#include <cstring>

#include "kfsys_interface.h"
#include "util.h"

using namespace std;

inline float unpack_fl (char* buf) {
    int itemp = ntohl(*(long*) buf);
    return *(float*) &itemp;
}

Libaave *KFSystem::libaave = NULL;

KFSystem::KFSystem() {
    if (!libaave) {
        libaave = new Libaave();  //init aave in this contructor.
    }
    audio_engine = 0; // default audio engine = direct sound
	global_position = 0;
	delay = 0;
}

void KFSystem::set_audio_engine(short ae) {

	audio_engine = ae;
}

void KFSystem::render(short *buff, int frames) {
    
    int i;
    
	memset(buff, 0, 2 * frames * sizeof(short));

	if (audio_engine == 1)
		libaave->update_geometry(); /* updates geometries + sources */

    for (i=0; i<sources.size(); i++) {
        if (audio_engine == 0)
			sources[i]->render(global_position, buff, frames);
        if (audio_engine == 1) {
			sources[i]->render(global_position, buff, frames);
			buffer_stereo_to_mono(buff, frames*2);
			libaave->put_audio(sources[i]->aave_source, buff, frames);
        }
    }

	if (audio_engine == 1)
		libaave->get_binaural_audio(buff, frames);

	global_position += frames;
}

void KFSystem::start_keyframes(int delay) {
    printf("starting with delay %i\n", delay);
    int i;
    for (i=0; i<sources.size(); i++) {
        sources[i]->start_keyframes(this, delay);
    }
}

int KFSystem::done() {
    int i;
    for (i=0; i<sources.size(); i++) {
        if (!sources[i]->done())
            return 0;
    }
    return 1;
}

short KFSystem::cmds_output_iterate(char *recvbuf, int recv_len) {
	
	return 2;
}

short KFSystem::cmds_output_set_frame(char *recv_buf, int recv_len) {
    
    start_keyframes(delay);
    
//    int start_frame = ntohl(*(long*)(recv_buf + 3)); // ignore it
    //delay += ntohs(*(short*)(recv_buf + 7));
    //printf("delay from net: %i\n", delay);
    return 6;
}

short KFSystem::cmds_listener_set_position(char *recv_buf, int recv_len) {

	libaave->set_listener_position(unpack_fl(recv_buf+1),unpack_fl(recv_buf+5),unpack_fl(recv_buf+9));
	libaave->set_listener_orientation(unpack_fl(recv_buf+13),unpack_fl(recv_buf+17),unpack_fl(recv_buf+21));
	return 25;
}

short KFSystem::handle_input_params_cmds(char *recv_buf, int recv_len) {
	short temp;
	switch (static_cast<input_params_cmds>(recv_buf[1]))
	{
		case input_params_cmds::mode:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("mode = %d\n",temp);
			break;
		case input_params_cmds::hrtf:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("HRTF = %hu\n", temp);
			if (temp == 1)
				aave_hrtf_mit(libaave->aave);
			if (temp == 2)
				aave_hrtf_cipic(libaave->aave);
			if (temp == 3)
				aave_hrtf_listen(libaave->aave);
			if (temp == 4)
				aave_hrtf_tub(libaave->aave);
			break;
		case input_params_cmds::reflections:
			temp = ntohs(*(short*)(recv_buf + 2));
			libaave->aave->reflections = temp;
			break;
		case input_params_cmds::frame_rate:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("frame rate = %d\n",temp);
			break;
		case input_params_cmds::audio_engine:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("audio engine = %d\n",temp);
			audio_engine = temp;
			break;
	}
	return 4;
}

short KFSystem::handle_reverb_cmds(char *recv_buf, int recv_len) {
	short temp, retv=0;
	
	switch (static_cast<reverb_cmds>(recv_buf[1]))
	{
		case reverb_cmds::add:
			aave_reverb_init(libaave->aave);
			retv = -2;
			break;
		case reverb_cmds::area:
			temp = ntohs(*(short*)(recv_buf + 2));
			aave_reverb_set_area(libaave->aave, temp);
			break;
		case reverb_cmds::volume:
			temp = ntohs(*(short*)(recv_buf + 2));
			aave_reverb_set_volume(libaave->aave, temp);
			break;
		case reverb_cmds::rt60:
			temp = ntohs(*(short*)(recv_buf + 2));
			aave_reverb_set_rt60(libaave->aave, temp);
			break;
		case reverb_cmds::reverb_active:
			temp = ntohs(*(short*)(recv_buf + 2));
			libaave->aave->reverb_active = temp;
			break;
		case reverb_cmds::reverb_mix:
			temp = ntohs(*(short*)(recv_buf + 2));
			libaave->aave->reverb->mix = temp;
			break;
	}
	retv +=4;
	return retv;
}

short KFSystem::handle_geometry_cmds(char *recv_buf, int recv_len) {
	short temp, retv=0;
	
	switch (static_cast<geometry_cmds>(recv_buf[1]))
	{
		case geometry_cmds::add:
			
			retv = 2;
			break;
		case geometry_cmds::set_geometry:
			retv = libaave->set_geometry(recv_buf, recv_len);
			break;
		case geometry_cmds::set_posrot:

			break;
		case geometry_cmds::set_material:
			retv = libaave->set_geometry_material(recv_buf, recv_len);
			break;
	}
	return retv;
	
}

short KFSystem::add_source(char *recv_buf, int recv_len) {
	
	// cmd recebe id da fonte a adicionar.
	// testar sinal do id, testar a existencia do id e, em caso positivo, ignorar inserção.
    printf("adding source: %d\n", recv_buf[2]);
    auto source = new Source;

    if (sources.find(recv_buf[2]) == sources.end()) {
		if (audio_engine==1)
			source->init_aave(libaave);
		sources.insert(make_pair(recv_buf[2], source));
		printf("insert src %d , sources size = %d\n", recv_buf[2], (int) sources.size());
	}
	return 3;
}

void KFSystem::handle_datagram(char *recv_buf, int recv_len) {
	int inc = 0;
	while (recv_len > inc) {
	    printf("KFSystem::handle_datagram cmd=%d (%d bytes left) inc=%d\n", recv_buf[inc], recv_len-inc, inc);
	    switch(static_cast<kfsys_cmds>(recv_buf[inc]))
        {
            case kfsys_cmds::listener:
            	inc += cmds_listener_set_position(recv_buf + inc, recv_len);
                break;                
            case kfsys_cmds::source:
            
            	if (sources.empty() && recv_buf[inc+1] != 0) {
            		printf("It is necessary to add a source before performing any other source command...\n");
            		return;
            	}
            	
            	switch (static_cast<source_cmds>(recv_buf[inc+1]))
            	{            		
                	case source_cmds::add:
						inc += add_source(recv_buf, recv_len);
				        break;
                 	default:
                		printf("handling source cmd for src %d\n",recv_buf[inc+2]);
                    	inc += sources[recv_buf[inc+2]]->handle_datagram(recv_buf + inc, recv_len);
                    	break;               
                }
                break; 
            case kfsys_cmds::geometry:
            	inc += handle_geometry_cmds(recv_buf + inc, recv_len);
                break;
            case kfsys_cmds::reverb:
            	inc += handle_reverb_cmds(recv_buf + inc, recv_len);
                break;         
            case kfsys_cmds::input_params:
				inc += handle_input_params_cmds(recv_buf + inc, recv_len);
                break;
            case kfsys_cmds::output:            
            	switch(static_cast<output_cmds>(recv_buf[inc+1])) {
			        case output_cmds::iterate:
			        	inc += cmds_output_iterate(recv_buf + inc, recv_len);
			            break;
			        case output_cmds::set_frame:
			        	inc += cmds_output_set_frame(recv_buf + inc, recv_len);
			            break;	
            	}            	
                break;          
            default:
                printf("Unrecognize command...\n\n");
                inc = recv_len;
        }
        printf("packet increment %d, recv_len = %d\n", inc, recv_len);
    }
}
