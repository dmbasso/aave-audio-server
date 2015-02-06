#include <fstream>

void init_output_wavfile(std::ofstream *ofs, int data_size);
void buffer_stereo_to_mono(short *buff, int len);
