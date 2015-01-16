#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

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

void write_frames(KFSystem *sys, short buff[], FILE* out, int* file_count) {

	string file_name;
	
	if (sys->write_frames>0) {
		fwrite(buff, sizeof(short)*2, BUFFLEN, out);
		sys->write_frames--;

		if (sys->write_frames==0) {
			fclose(out);
			*file_count += 1;
			printf("file count = %d\n", *file_count);

			file_name.clear();
			file_name.append("../sounds/output/out_udp");
			file_name.append(std::to_string(*file_count));
			file_name.append(".raw");

			out = fopen(file_name.c_str(), "w");
			sys->write_frames--;
		}
	}
}

int main() {//_latency_check() {

    int socket = socket_init();
    struct sockaddr_in addr;
    socklen_t slen;
    
    short buff[BUFFLEN * 2];

    string file_name;
    int file_count = 1;

    KFSystem sys;    
    Alsa alsa;

	alsa.setup_default();   
    
    int recv_len, avail;
    char *recv_buff;
    recv_buff = (char *) malloc(8192);

	/* Aave latency compensation. */
	//delay += 333;  //mit
    //sys.delay = delay;
	//delay += -598;  //cipic
	//delay += -1124; //listen
	//delay += -2169; //tub

	file_name.append("../sounds/output/out_udp");
	file_name.append(std::to_string(file_count));
	file_name.append(".raw");
    FILE *out = fopen(file_name.c_str(),"w");

    printf("waiting for udp packet at port %i\n", PORT);
    
    while (1) {

        recv_len = recvfrom(socket, recv_buff, 8192, 0, (struct sockaddr*) &addr, &slen);
        
        if (recv_len > 0)        	
        	sys.handle_datagram(recv_buff, recv_len);
        
        sys.render(buff, BUFFLEN);
        write_frames(&sys, buff, out, &file_count);
        alsa.write(buff, BUFFLEN);        
	}
    alsa.shutdown();
}
