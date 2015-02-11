#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> //abort()
#include <cstring>

#include "kfsys_interface.h"
#include "util.h"
#include "test.h"

using namespace std;

inline float unpack_fl (char* buf) {
    int itemp = ntohl(*(long*) buf);
    return *(float*) &itemp;
}

Libaave *KFSystem::libaave = NULL;

KFSystem::KFSystem() {

    if (!libaave)
        libaave = new Libaave();  //init aave in this contructor.

    audio_engine = 0; // default audio engine = direct sound
    write_frames = -1;
	global_position = 0;
	delay = 0;
    mode = processing_modes::realtime;
    started = false;
}

struct aave* KFSystem::get_aave_engine() {	
	if (libaave->aave)
		return libaave->aave;
	return NULL;	
}

struct aave_surface* KFSystem::get_aave_surfaces() {	
	if (libaave->aave->surfaces)
		return libaave->aave->surfaces;
	return NULL;	
}

void KFSystem::set_audio_engine(short ae) {

	audio_engine = ae;
}

short KFSystem::set_aave_hrtf(short hrtf) {
	
	int aave_delay;
	
	if (hrtf == 1) {
		aave_hrtf_mit(get_aave_engine());
		aave_delay = -333;
	}
	if (hrtf == 2) {
		aave_hrtf_cipic(get_aave_engine());
		aave_delay = -598;
	}
	if (hrtf == 3) {
		aave_hrtf_listen(get_aave_engine());
		aave_delay = -1124;
	}
	if (hrtf == 4) {
		aave_hrtf_tub(get_aave_engine());
		aave_delay = -2169;
	}
	return hrtf;
}

void KFSystem::set_listener_position(float x, float y, float z) {	
	libaave->set_listener_position(x, y, z);	
}

void KFSystem::set_listener_orientation(float roll, float pitch, float yaw) {	
	libaave->set_listener_orientation(roll, pitch, yaw);	
}

float* KFSystem::get_listener_position() {	
	return libaave->get_listener_position();	
}

void KFSystem::set_reflection_order(unsigned n) {
	libaave->set_reflection_order(n);	
}

unsigned KFSystem::get_reflection_order() {
	return libaave->get_reflection_order();	
}

void KFSystem::update_aave_geometry() {	
	libaave->update_geometry();
}

void KFSystem::init_aave_reverb() {	
	libaave->init_reverb();
}

void KFSystem::set_aave_reverb_rt60(unsigned short rt60) {
	libaave->set_reverb_rt60(rt60);
}

void KFSystem::set_aave_reverb_area(unsigned short area) {
	libaave->set_reverb_area(area);
}

void KFSystem::set_aave_reverb_volume(unsigned short volume) {
	libaave->set_reverb_volume(volume);
	aave_reverb_print_parameters(get_aave_engine(), get_aave_engine()->reverb);
}

void KFSystem::enable_disable_aave_reverb() {
	libaave->enable_disable_reverb();
}

void KFSystem::set_aave_gain(float gain) {
	libaave->set_gain(gain);
}

void KFSystem::increase_aave_gain() {
	libaave->increase_gain();
}

void KFSystem::decrease_aave_gain() {
	libaave->decrease_gain();
}

void KFSystem::add_source(int id) {
	
	// cmd recebe id da fonte a adicionar.
	// testar sinal do id, testar a existencia do id e, em caso positivo, ignorar inserção.    
    auto source = new Source;

    if (sources.find(id) == sources.end()) {
		if (audio_engine==1)
			source->init_aave(libaave);
		sources.insert(make_pair(id, source));
		printf("\ninsert src %d , sources size = %d\n", id, (int) sources.size());
	}
	else printf("Source %d already allocated...\n", id);	
}

struct aave_source* KFSystem::get_aave_source(short id) {

	if (sources.find(id) != sources.end())
		return sources.at(id)->aave_source;
	return NULL;
}

short KFSystem::cmds_add_source(char *recv_buf, int recv_len) {   
    add_source(recv_buf[2]);
	return 3;
}

void KFSystem::render(short *buff, int frames) {
    
	memset(buff, 0, 2 * frames * sizeof(short));

	if (audio_engine == 1)
		libaave->update_geometry(); /* updates geometries + sources */

    for (unsigned i=0; i<sources.size(); i++) {
        if (audio_engine == 0)
			sources[i]->render(global_position, buff, frames);
        if (audio_engine == 1) {
			sources[i]->render(global_position, buff, frames);
			for (unsigned j=0; j<frames; j++) {
			    buff[j] = ((int)buff[j * 2] + buff[j * 2 + 1]) >> 2;
			}
			libaave->put_audio(sources[i]->aave_source, buff, frames);
            memset(buff, 0, 2 * frames * sizeof(short));
        }
    }

	if (audio_engine == 1)
		libaave->get_binaural_audio(buff, frames);

	global_position += frames;
}

void KFSystem::start_keyframes(int delay) {
    printf("starting with delay %i\n", delay);
    for (unsigned i=0; i<sources.size(); i++) {
        sources[i]->start_keyframes(this, delay);
    }
}

int KFSystem::done() {
    for (unsigned i=0; i<sources.size(); i++) {
        if (!sources[i]->done())
            return 0;
    }
    return 1;
}

short KFSystem::cmds_output_set_frame(char *recv_buf, int recv_len) {

    //hack
    set_aave_gain(10);
	get_aave_engine()->reverb->level = 0.3;
    
	//int start_frame = ntohl(*(long*)(recv_buf + 2)); // ignore it
    delay = ntohl(*(long*)(recv_buf + 6));
    start_keyframes(delay);
    printf("delay from net: %i\n", delay);
    return 10;
}

short KFSystem::cmds_output_write_frames(char *recv_buf, int recv_len) {

    start_keyframes(delay);

	int nframes = ntohl(*(int32_t*)(recv_buf + 2)); // ignore it
	write_frames = nframes;

    printf("%d frames to be written\n", nframes);
    return 6;
}

short KFSystem::cmds_output_iterate(char *recvbuf, int recv_len) {
    #ifndef BUFFLEN
        #define BUFFLEN 2048
    #endif
    int nframes = write_frames;

    #define BUFFSIZE (BUFFLEN * 2 * 2)  // memory size of the buffer in bytes
	int data_size = nframes * BUFFSIZE; // 16 bit stereo frames
    short buff[BUFFLEN * 2];
    memset(buff, 0, BUFFLEN * 2);

    std::ofstream ofs;

    // TODO the output path must be a parameter, not be hardcoded
    ofs.open(("output.wav"), std::ofstream::out);
    init_output_wavfile(&ofs, data_size);

    while (nframes + BUFFLEN > 0) {
		render(buff, BUFFLEN);
		ofs.write((char *) &buff, BUFFSIZE > nframes ? nframes : BUFFSIZE);
		nframes -= BUFFLEN;
		// recv iterate
    }
    ofs.close();
    printf("%d frames written to %s\n", write_frames, "output.wav"); // TODO

	return 2;
}

short KFSystem::cmds_listener_set_position(char *recv_buf, int recv_len) {

	set_listener_position(unpack_fl(recv_buf+1),unpack_fl(recv_buf+5),unpack_fl(recv_buf+9));
	set_listener_orientation(unpack_fl(recv_buf+13),unpack_fl(recv_buf+17),unpack_fl(recv_buf+21));
	printf("listener position = %.2f - %.2f - %.2f\n", unpack_fl(recv_buf+1), unpack_fl(recv_buf+5), unpack_fl(recv_buf+9));
	return 25;
}

short KFSystem::handle_input_params_cmds(char *recv_buf, int recv_len) {
	short temp;
	switch (static_cast<input_params_cmds>(recv_buf[1]))
	{
		case input_params_cmds::mode:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("mode = %d\n",temp);
			started = true;
			break;
		case input_params_cmds::hrtf:
			temp = ntohs(*(short*)(recv_buf + 2));
			printf("HRTF = %hu\n", temp);
			set_aave_hrtf(temp);
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
			init_aave_reverb();
			retv = -2;
			break;
		case reverb_cmds::area:
			temp = ntohs(*(short*)(recv_buf + 2));
			set_aave_reverb_area(temp);
			break;
		case reverb_cmds::volume:
			temp = ntohs(*(short*)(recv_buf + 2));
			set_aave_reverb_volume(temp);
			break;
		case reverb_cmds::rt60:
			temp = ntohs(*(short*)(recv_buf + 2));
			set_aave_reverb_rt60(temp);
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

	short retv=0;
	
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

void KFSystem::handle_datagram(char *recv_buf, int recv_len) {
	int inc = 0;
	while (recv_len > inc) {
	    //printf("KFSystem::handle_datagram cmd=%d (%d bytes left) inc=%d\n", recv_buf[inc], recv_len-inc, inc);
	    switch(static_cast<kfsys_cmds>(recv_buf[inc]))
        {
            case kfsys_cmds::listener:
            	inc += cmds_listener_set_position(recv_buf + inc, recv_len);
                break;                
            case kfsys_cmds::source:
            
            	if (sources.empty() && recv_buf[inc+1] != 0) {
            		printf("It is necessary to add a source before performing any other source related command...\n");
            		return;
            	}
            	
            	switch (static_cast<source_cmds>(recv_buf[inc+1]))
            	{            		
                	case source_cmds::add:
						inc += cmds_add_source(recv_buf, recv_len);
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
					case output_cmds::write_frames:
						inc += cmds_output_write_frames(recv_buf + inc, recv_len);
			            break;	
            	}            	
                break;          
            default:
                printf("Unrecognize command...\n\n");
                inc = recv_len;
        }
        //printf("packet increment %d, recv_len = %d\n", inc, recv_len);
    }
}
