#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string>

#include "aave_interface.h"

using namespace std;

inline float unpack_fl (char* buf) {
    int itemp = ntohl(*(long*) buf);
    return *(float*) &itemp;
}

Libaave::Libaave() {

    aave = (struct aave *) malloc(sizeof *aave);
    aave->gain = 1;
    aave->nsurfaces = 0;
    aave->reverb_active = 0;
    aave->reflections = 0;
    
    /* Select the HRTF set to use. */
	aave_hrtf_mit(aave);
	/* aave_hrtf_cipic(aave); */
	/* aave_hrtf_listen(aave); */
	/* aave_hrtf_tub(aave); */
}

void Libaave::set_listener_position(float x, float y, float z) {
	aave_set_listener_position(aave, x, y, z);
}

float* Libaave::get_listener_position() {
	float* pos = (float*) malloc(sizeof(float) * 3);
	pos[0] = aave->position[0];
	pos[1] = aave->position[1];
	pos[2] = aave->position[2];
	return pos;
}

void Libaave::set_listener_orientation(float x, float y, float z) {
	aave_set_listener_orientation(aave, x, y, z);
}

short Libaave::set_geometry(char *recv_buf, int recv_len) {

	float vertices[MAX_VERTICES][3];
	struct aave_surface *surface;
	unsigned short geom, nverts, nfaces, vertex_count, face_count, vert_size, vertex_index;
    unsigned short i;

	printf("SETTING GEOMETRY:\n");
	
	printf("geom = %d, len_vet = %d, len_faces = %d\n",recv_buf[2],recv_buf[3],recv_buf[4]);
    geom = recv_buf[2];
    nverts = recv_buf[3];
    nfaces = recv_buf[4];        

    vertex_count=0;
    i=5; 
    for (vertex_count=0;vertex_count < nverts; vertex_count++) {
        vertices[vertex_count][0] = unpack_fl(&recv_buf[i]);
        vertices[vertex_count][1] = unpack_fl(&recv_buf[i+4]);
	    vertices[vertex_count][2] = unpack_fl(&recv_buf[i+8]);
	    i+=12;               
    }
    
    vert_size = recv_buf[i];
    
    for (face_count=0;face_count < nfaces;face_count++) {                
        surface = (struct aave_surface*) malloc(sizeof *surface);
        surface->npoints = 0;
        surface->geometry = geom;
        		                            
        vert_size = recv_buf[i];
        i += 1;
        for (vertex_count=0; vertex_count < vert_size; vertex_count++) {
        	vertex_index = (int) recv_buf[i];
  			printf("adding vertex %d = %f , %f , %f\n",vertex_index,vertices[vertex_index][0],vertices[vertex_index][1],vertices[vertex_index][2]);                  
            surface->points[surface->npoints][0] = vertices[vertex_index][0];
            surface->points[surface->npoints][1] = vertices[vertex_index][1];
            surface->points[surface->npoints][2] = vertices[vertex_index][2];
            surface->npoints++;
            i += 1;
        }
        aave_add_surface(aave, surface);
    }   
    return i;      
}

short Libaave::set_geometry_material(char *recv_buf, int recv_len) {

	struct aave_surface *surface;
	unsigned geom = (unsigned char) recv_buf[2];
	int len = (unsigned char) recv_buf[3];
	int j;
    string material_name(&recv_buf[4], len);
    printf("setting material name = %s\n", material_name.c_str());
    
	surface = aave->surfaces;

	while(surface) {
		if (surface->geometry == geom) {
		    //printf("Setting material %s for Geom %u\n",material_name,surface->geometry);                
		    surface->material = aave_get_material(material_name.c_str());
		    surface->avg_absorption_coef = 0;

		    for (j = 0; j < AAVE_MATERIAL_REFLECTION_FACTORS; j++) 
			    surface->avg_absorption_coef += (0.01 * surface->material->reflection_factors[j]) / AAVE_MATERIAL_REFLECTION_FACTORS;
		}
		surface = surface->next;               
	}      
    return 3 + 1 + len;
}

void Libaave::update_geometry() {
	aave_update(aave);
}

void Libaave::put_audio(struct aave_source* src, short * buff, int bufflen) {
	aave_put_audio(src, buff, bufflen); 
}

void Libaave::get_binaural_audio(short * buff, int bufflen) {
	aave_get_audio(aave, buff, bufflen); 
}

void Libaave::init_reverb() {
	aave_reverb_init(aave);
}

void Libaave::set_reverb_rt60(unsigned short rt60) {
	aave_reverb_set_rt60(aave, rt60);
}

void Libaave::set_reverb_area(unsigned short area) {
	aave_reverb_set_area(aave, area);
}

void Libaave::set_reverb_volume(unsigned short volume) {
	aave_reverb_set_volume(aave, volume);
}

void Libaave::enable_reverb() {
	aave->reverb_active = 1;
}
void Libaave::disable_reverb() {
	aave->reverb_active = 0;
}

void Libaave::set_gain(float gain) {
	aave->gain = gain;
}

void Libaave::increase_gain() {
	aave->gain *= 1.5;
}

void Libaave::decrease_gain() {
	aave->gain /= 1.5;
}
