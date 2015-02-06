#!/usr/bin/env python
#-*- coding:utf-8 -*-

import sys
import time
import math
from client import AudioServerInterface, Object, PY3

obj = Object("../geometries/model.obj")
srv = AudioServerInterface()
srv.debug = "-d" in sys.argv
#srv.debug = True

VOLUME = 4500
AREA = 1720
RT60 = 2000
NSOURCES = 9

# source sound files
sound_files = []
sound_file_path = '../sounds/input/'
sound_file_type = '.wav'
sound_files.append('cello')
sound_files.append('clarinet')
sound_files.append('double_bass')
sound_files.append('flute')
sound_files.append('harp')
sound_files.append('oboe')
sound_files.append('percussion')
sound_files.append('trombone')
sound_files.append('violin')

# source positions for adding keyframes
pos = []
pos.append((6,-2,1))	# cello
pos.append((-4,2,1))	# clarinet
pos.append((11,-2,1))	# double_bass
pos.append((0,0,1))		# flute
pos.append((-11,-2,1))	# harp
pos.append((4,2,1))		# oboe
pos.append((-11,2,1))	# percussion
pos.append((11,2,1))	# trombone
pos.append((-6,-2,1))	# violin

# initial delay
pause = 0 # msecs

for i in range(3):
    # set input parameters
    with srv.packet() as pkt:
	    pkt.set_input_param(pkt.input_param.audio_engine, 1)
	    pkt.set_input_param(pkt.input_param.hrtf, pkt.hrtf.listen)
	    pkt.set_input_param(pkt.input_param.reflections,0)
	    pkt.set_input_param(pkt.input_param.mode, pkt.modes.realtime)

    # set listener position and orientation
    with srv.packet() as pkt:
        pkt.set_listener((0, -5, 1), (0, 0, -math.pi/2.))

    # set scene geometry
    with srv.packet() as pkt:
        for geometry in obj.geometries:
            geom = pkt.add_geometry()
            pkt.set_geometry(geom, obj.vertices, geometry[0])
            pkt.set_geometry_material(geom, geometry[1])

    # add and set reverb params
    with srv.packet() as pkt:
	    pkt.add_reverb()
	    pkt.set_reverb_param(pkt.reverb_cmds.area, AREA)
	    pkt.set_reverb_param(pkt.reverb_cmds.volume, VOLUME)
	    pkt.set_reverb_param(pkt.reverb_cmds.rt60, RT60)
        
    for i in range(NSOURCES):
	    with srv.packet() as pkt:
		    src = pkt.add_source()
		    pkt.set_sound_file(src, sound_file_path + sound_files[i] + sound_file_type)
		    pkt.clear_keyframes(src)
		    pkt.add_keyframe(src, pause, pos=pos[i])

#time.sleep(5)

#with srv.packet() as pkt:
#	pkt.output_set_frame(0)
	#pkt.output_write_frames(100)
	#time.sleep(5)
#	break

#with srv.packet() as pkt:
#    pkt.output_set_frame(-1)  # stop keyframes

