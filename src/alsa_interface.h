#include <alsa/asoundlib.h>

class Alsa {
	public:
		snd_pcm_t *alsa_handle;

        Alsa() { alsa_handle=NULL; }
        operator bool () { return alsa_handle != NULL; }
		void setup(int rate, int channels, snd_pcm_uframes_t frames);
		void setup_default();
		void shutdown();
		void write(void *buff, int frames);
		int  avail();
		int  delay();
};
