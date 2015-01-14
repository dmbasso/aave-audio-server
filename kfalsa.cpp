#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "kfsys_interface.h"
#include "aave_interface.h"
#include "alsa_interface.h"


#define PORT 34492


int socket_init()
{
    struct sockaddr_in addr;
    int s;
    socklen_t slen;

    /* Create a UDP socket. */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("socket");
        exit(1);
    }

    fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));     
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
     
    /* Bind socket to port. */
    if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        printf("bind");
        exit(1);
    }
    return s;
}


int main() {//_latency_check() {

    int socket = socket_init();
    short buff[BUFFLEN * 2];
    memset(buff, 0, BUFFLEN * 2);

    KFSystem sys;
    int i;
    
    Alsa alsa;
    
	//sys.libaave->set_geometry("model.obj");
	sys.libaave->set_listener_position(0, 0, 0);
	sys.libaave->set_listener_orientation(0, 0, 0);
	
    /*	
    for (i=0; i<2; i++) {
        snd[i].init(i, "dummy.snd");
    }
    */
    /*
    for (i=0; i<SOURCES; i++) {
        sys.sources[i]->set_sound(&snd[i]);
        sys.sources[i]->add_source(aave);
        sys.sources[i]->set_source_position(0, 0, 0);        
        sys.sources[i]->set_default_keyframes();
    }*/
    
    //sys.libaave->update_geometry(); /* updates geometries + sources */


	/* error in this initialisation block. ignored by snd_pcm_open, snd_pcm_set_params */ 
    alsa.setup(44100, 2, 8192);
    int alsa_bufflen = alsa.avail();
    printf("bufflen: %i\n", alsa_bufflen);
    
    
    int64_t delay;
    socklen_t slen;
    int recv_len, avail;
    char *recv_buff;
    recv_buff = (char *) malloc(8192);
    struct sockaddr_in addr;
    
    struct timeval tim;
    gettimeofday(&tim, NULL);
    printf("tstamp: %lf\n", tim.tv_sec+(tim.tv_usec/1000000.0));

    
	/* Aave latency compensation. */
	//delay += 333;  //mit
    //sys.delay = delay;
	//delay += -598;  //cipic
	//delay += -1124; //listen
	//delay += -2169; //tub

    FILE *out = fopen("out.raw", "wb");
    //int first_packet = 0;

    printf("waiting for udp packet at port %i\n", PORT);
    //int cnt = 44100 * 10 / 1024;
    while (1) {

        recv_len = recvfrom(socket, recv_buff, 8192,
                            0, (struct sockaddr*) &addr, &slen);
        if (recv_len > 0) {
            int64_t video_frame_period = 10 * 16000; // microseconds
            int64_t other_latencies = 0; // microseconds
            video_frame_period -= other_latencies;
            delay = video_frame_period * 44100 / 1000000; // frames
            //printf("frames until video unlock: %lu\n", delay);
            alsa.avail(); // update internal ALSA status buffers
            int t = alsa.delay();
            printf("alsa.delay: %i\n", t);
            delay -= t;
            sys.delay = delay;
            printf("delay: %li samples\n", delay);

            printf("PKTLEN %i\n", recv_len);
        	sys.handle_datagram(recv_buff, recv_len);
            /*first_packet = 1;
        	usleep(100000);
        	continue;*/
    	}
    	/*if (!first_packet) {
        	usleep(100000);
        	continue;
    	}*/
        avail = alsa.avail();
        if (avail > BUFFLEN) {
            sys.render(buff, BUFFLEN);
            //fwrite(buff, 2, BUFFLEN * 2, out);
            /*if (first_packet && !sys.done()) {
            	printf("written\n");
                cnt--;
           }*/
            alsa.write(buff, BUFFLEN);
             //we should keep it almost empty
        }
    }
    alsa.shutdown();
}
