#!/usr/bin/env python
#-*- coding:utf-8 -*-

# To play the output .raw file using aplay run:
# aplay -t raw -c 2 -f cd -r 44100 output.raw

import testlib as tl

tl.add_source(0)
tl.set_source_sound(0, "foot.wav")
tl.set_source_position(0, 5., 5., 0.)

# still not processing kf positions (aave)
tl.source_add_keyframe(0,  500, 0, 5.,  5., 0.)
tl.source_add_keyframe(0, 1000, 0, 5.,  0., 0.)
tl.source_add_keyframe(0, 2000, 0, 5., -5., 0.)

tl.start_keyframes(0)
tl.render_nframes(22)
