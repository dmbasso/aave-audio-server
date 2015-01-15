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
    struct sockaddr_in addr;
    socklen_t slen;
    
    short buff[BUFFLEN * 2];
    memset(buff, 0, BUFFLEN * 2);

    KFSystem sys;    
    Alsa alsa;

//    alsa.setup(44100, 2, 8192);
//    int alsa_bufflen = alsa.avail();
//    printf("bufflen: %i\n", alsa_bufflen);

	alsa.setup_default();   
    
    int recv_len, avail;
    char *recv_buff;
    recv_buff = (char *) malloc(8192);
    
	sys.audio_engine = 1;
    
	/* Aave latency compensation. */
	//delay += 333;  //mit
    //sys.delay = delay;
	//delay += -598;  //cipic
	//delay += -1124; //listen
	//delay += -2169; //tub

    FILE *out = fopen("../sounds/output/out_udp.raw", "wb");
    //int first_packet = 0;

    printf("waiting for udp packet at port %i\n", PORT);
    //int cnt = 44100 * 10 / 1024;
    while (1) {

        recv_len = recvfrom(socket, recv_buff, 8192, 0, (struct sockaddr*) &addr, &slen);
        
        if (recv_len > 0)        	
        	sys.handle_datagram(recv_buff, recv_len);
        
        sys.render(buff, BUFFLEN);
        alsa.write(buff, BUFFLEN);        
    }
    alsa.shutdown();
}
