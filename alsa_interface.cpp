#include <stdio.h>

#include "alsa_interface.h"

void Alsa::setup_default() {

	const char device[] = "default";
	int err;

	/* Open the ALSA audio device. Use default system sound output. */
	err = snd_pcm_open(&alsa_handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_open: %s\n", snd_strerror(err));
		return;
	}
	err = snd_pcm_set_params(alsa_handle, SND_PCM_FORMAT_S16,
				SND_PCM_ACCESS_RW_INTERLEAVED,
				2, 44100, 0, 100000); /* 100ms latency */
	if (err < 0) {
		fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(err));
		return;
	}
}

void Alsa::setup(int rate, int channels, snd_pcm_uframes_t frames)
{
    const char *device="default";
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	int err, dir;

	if ((err = snd_pcm_open (&alsa_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 device,
			 snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (alsa_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (alsa_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (alsa_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (alsa_handle, hw_params, (unsigned int *) &rate, &dir)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
    printf("sampling rate: %i\n", rate);


	if ((err = snd_pcm_hw_params_set_channels (alsa_handle, hw_params, channels)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_period_size_near (alsa_handle, hw_params, &frames, &dir)) < 0) {
		fprintf (stderr, "cannot set period size (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	//printf("MOOO %lu\n", frames);
    
	if ((err = snd_pcm_hw_params (alsa_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);
}


void Alsa::shutdown()
{
	snd_pcm_close (alsa_handle);
}


void Alsa::write(void *buff, int frames)
{
    int err;
    err = snd_pcm_writei(alsa_handle, buff, frames);
    if (err != frames) {
        fprintf(stderr, "snd_pcm_writei: %s\n",snd_strerror (err));
        exit(1);
    }
}

int Alsa::avail()
{
    return snd_pcm_avail(alsa_handle);
}

int Alsa::delay()
{
    int err;
 	snd_pcm_sframes_t frames;
    if ((err = snd_pcm_delay(alsa_handle, &frames)) < 0) {
        fprintf(stderr, "snd_pcm_writei: %s\n",snd_strerror (err));
        exit(1);
    }
    return frames;
}

/*int main()
{
    int err;
    alsa_setup(44100, 1);
    struct timeval tim;
	double t_current, t_previous;
	snd_pcm_uframes_t frames;
	while(1) {
	    err = snd_pcm_writei(alsa_handle, buf, FRAMES);
	    if (err != FRAMES) {
	        fprintf(stderr, "snd_pcm_writei: %s\n",snd_strerror (err));
	        break;
	    }
	    err = snd_pcm_writei(alsa_handle, buf, FRAMES);
	    if (err != FRAMES) {
	        fprintf(stderr, "snd_pcm_writei: %s\n",snd_strerror (err));
	        break;
	    }
		if (err = snd_pcm_delay(alsa_handle, &frames) < 0) {
	        fprintf(stderr, "snd_pcm_writei: %s\n",snd_strerror (err));
	        break;
		}
		printf("%i\n", frames);
	    gettimeofday(&tim, NULL);  
        t_current = (tim.tv_sec+(tim.tv_usec/1000000.0));
        printf("%d ms\n", (int) ((t_current - t_previous) * 1000));           
        t_previous = t_current; 
		int i;
		for (i=0; i<20; i++) {
			int f = snd_pcm_avail(alsa_handle);
			printf("f: %i\n", f);
		    usleep(5000);
		}
    }   
    alsa_shutdown();
}*/
