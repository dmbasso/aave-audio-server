#!/usr/bin/env python
#-*- coding:utf-8 -*-

from ctypes import *

testlib = cdll.LoadLibrary("./_testlib.so")

add_source = testlib.add_source
add_source.argtypes = [c_int]

set_source_sound = testlib.set_source_sound
set_source_sound.argtypes = [c_int, c_char_p]

set_source_position = testlib.set_source_position
set_source_position.argtypes = [c_int, c_double, c_double, c_double]

source_start_sound = testlib.source_start_sound
source_start_sound.argtypes = [c_int]

source_add_keyframe = testlib.source_add_keyframe
source_add_keyframe.argtypes = [c_int, c_int, c_int, c_double, c_double, c_double]
#(int id, int start,	int flags, float posx, float posy, float posz);

start_keyframes = testlib.start_keyframes
start_keyframes.argtypes = [c_int]

render_nframes = testlib.render_nframes
render_nframes.argtypes = [c_int]
#(int nframes)

