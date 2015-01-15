/** To play the output .raw file using aplay run:
  * $aplay -t raw -c 2 -f cd -r 44100 output.raw */

#include "test.h"
#include "kfsys_interface.h"


int main() {

	set_audio_engine(1);
	set_hrtf(1);
	set_listener_position(0,0,0);
	set_listener_orientation(0,0,0);
	set_geometry("../geometries/model.obj");
	set_reflection_order(0);
	init_reverb();
	set_reverb_rt60(3000);
	set_reverb_area(4000);
	set_reverb_volume(4000);
	set_gain(0.7);

    //source_convert_stereo_to_mono(0);
    //source_write_sound_file(0);

	add_source(0);
    set_source_sound(0,"../sounds/input/foot.wav");
    set_source_position(0, 0., 0., 0.);
    source_clear_keyframes(0);

//    source_add_keyframe(0, 22050, 2, 0., -5., 0.);
//    source_add_keyframe(0, 44100, 2, 0., 0., 0.);
//    source_add_keyframe(0, 66050, 2, 0., 5., 0.);

	for (int i = 0; i<15 ; i++)
		source_add_keyframe(0, 22050 * i, 2, 1., float(-7 + i), 0.);

    start_keyframes(0);
    render_frames_todriver(200);
}
