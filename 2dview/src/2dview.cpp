/** To play the output .raw file using aplay run:
  * $aplay -t raw -c 2 -f cd -r 44100 output.raw */

#include "../src/test.h"
#include "../src/kfsys_interface.h"
#include "view.h"

#include <thread>
#include <string>
#include <vector>
#include <iostream>

#include <QApplication>

#define NSOURCES 2

using namespace std;

int main(int argc, char **argv) {

	string sound_file_path("../../sounds/input/");
	const char *sounds[] = {"cello", "clarinet","double_bass", "flute", "harp", "oboe", "percussion", "trombone", "violin"};
	vector<string> sound_files(sounds, sounds + NSOURCES);
	string file_type(".wav");

	float pos[NSOURCES][3];
	
	int second_row_distance = -6;
	int first_row_distance = -13;

//	pos[0][0] = 6; pos[0][1] = -2; pos[0][2] = 1;
//	pos[1][0] = -4; pos[1][1] = 2; pos[1][2] = 1;
//	pos[2][0] = 11; pos[2][1] = -2; pos[2][2] = 1;
//	pos[3][0] = 0; pos[3][1] = 0; pos[3][2] = 1;
//	pos[4][0] = -11; pos[4][1] = -2; pos[4][2] = 1;
//	pos[5][0] = 4; pos[5][1] = 2; pos[5][2] = 1;
//	pos[6][0] = -11; pos[6][1] = 2; pos[6][2] = 1;
//	pos[7][0] = 11; pos[7][1] = 2; pos[7][2] = 1;
//	pos[8][0] = -6; pos[8][1] = -2; pos[8][2] = 1;

	pos[0][0] = 12; pos[0][1] = first_row_distance; pos[0][2] = 1;
	pos[1][0] = -8; pos[1][1] = second_row_distance; pos[1][2] = 1;
	pos[2][0] = 25; pos[2][1] = first_row_distance; pos[2][2] = 1;
	pos[3][0] = 0; pos[3][1] = second_row_distance; pos[3][2] = 1;
	pos[4][0] = -21; pos[4][1] = first_row_distance; pos[4][2] = 1;
	pos[5][0] = 8; pos[5][1] = second_row_distance; pos[5][2] = 1;
	pos[6][0] = -20; pos[6][1] = second_row_distance; pos[6][2] = 1;
	pos[7][0] = 24; pos[7][1] = second_row_distance; pos[7][2] = 1;
	pos[8][0] = -8; pos[8][1] = first_row_distance; pos[8][2] = 1;

	set_audio_engine(1);
	set_hrtf(3);
	set_listener_position(0., -15., 1.);
	set_listener_orientation(0., 0., -M_PI/2);
	set_geometry("../../geometries/model.obj");
	set_reflection_order(1);
	init_reverb();
	set_reverb_rt60(2000);
	set_reverb_area(1720);
	set_reverb_volume(4500);
	set_gain(10); // not included in the comunication protocol
	
	get_aave_engine()->reverb->level = 0.3;

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
