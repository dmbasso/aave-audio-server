/** To play the output .raw file using aplay run:
  * $aplay -t raw -c 2 -f cd -r 44100 output.raw */

#include "test.h"
#include "kfsys_interface.h"
#include "view.h"

#include <thread>
#include <string>
#include <vector>
#include <iostream>

#include <QApplication>

#define NSOURCES 12

using namespace std;

int main(int argc, char **argv) {

	string sound_file_path("../../sounds/input/");
	const char *sounds[] = {"bassoon", "cello", "clarinet","double_bass", "ensemble_strings", "ensemble_strings_pizzicato", "flute", "harp", "oboe", "percussion", "trombone", "violin"};
	vector<string> sound_files(sounds, sounds + NSOURCES);
	string file_type(".wav");

	float pos[12][3];
	pos[0][0] = 4; pos[0][1] = 3; pos[0][2] = 1;
	pos[1][0] = 5; pos[1][1] = -4; pos[1][2] = 1;
	pos[2][0] = -4; pos[2][1] = 3; pos[2][2] = 1;
	pos[3][0] = 11; pos[3][1] = -1; pos[3][2] = 1;
	pos[4][0] = -7; pos[4][1] = -1; pos[4][2] = 1;
	pos[5][0] = 7; pos[5][1] = -1; pos[5][2] = 1;
	pos[6][0] = -2; pos[6][1] = 0; pos[6][2] = 1;
	pos[7][0] = -11; pos[7][1] = -4; pos[7][2] = 1;
	pos[8][0] = 2; pos[8][1] = 0; pos[8][2] = 1;
	pos[9][0] = 11; pos[9][1] = -4; pos[9][2] = 1;
	pos[10][0] = 0; pos[10][1] = 6; pos[10][2] = 1;
	pos[11][0] = -5; pos[11][1] = -4; pos[11][2] = 1;

	set_audio_engine(1);
	set_hrtf(4);
	set_listener_position(0., -7., 1.);
	set_listener_orientation(0., 0., -M_PI/2);
	set_geometry("../../geometries/model.obj");
	set_reflection_order(1);
	init_reverb();
	set_reverb_rt60(2500);
	set_reverb_area(1720);
	set_reverb_volume(4500);
	set_gain(5);

    //source_convert_stereo_to_mono(0);
    //source_write_sound_file(0);

	for (short i = 0; i < NSOURCES; i++) {
		add_source(i);
		string snd_file_path = sound_file_path + sound_files.at(i) + file_type;
		cout << "adding sound " << i << " = " << snd_file_path << endl;
		set_source_sound(i,snd_file_path.c_str());
		source_clear_keyframes(i);
		source_add_keyframe(i, 44100 * 3, 2, pos[i][0], pos[i][1], pos[i][2]);
    }

	start_keyframes(0);
	std::thread render_audio(render_frames_todriver,5000);

	QApplication app(argc, argv);
	app.setApplicationName("ON2 DEMO");
	View view;
    view.show();
    return app.exec();   
}
