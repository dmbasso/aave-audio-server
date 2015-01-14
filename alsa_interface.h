#include <alsa/asoundlib.h>

class Alsa {
	public:
		snd_pcm_t *alsa_handle;

		void setup(int rate, int channels, snd_pcm_uframes_t frames);
		void shutdown();
		void write(void *buff, int frames);
		int  avail();
		int  delay();
};
