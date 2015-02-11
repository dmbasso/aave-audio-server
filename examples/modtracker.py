#!/usr/bin/env python
#-*- coding:utf-8 -*-

import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "../client/"))

import xm
import numpy as np
from scipy.io import wavfile
from client import AudioServerInterface


wave_path = os.path.abspath("square.wav")


class AcousticAVEModuleRenderer:
    def __init__(self, module, server, sample_rate=44100, loops=1):
        self.mod = module
        self.srv = server
        self.sample_rate = sample_rate
        self.loops = loops
        # http://modarchive.org/forums/index.php?topic=2709.%200
        ticks_per_second = self.mod.bpm * 2. / 5
        self.row_length = 1 / ticks_per_second * self.mod.tempo
        self.pattern = 0
        self.row = -1
        self.channels = [
            Channel(self) for note in self.mod.patterns[0].rows[0].notes
        ]
        self.done = not self.mod.patterns
        self.time = 0

    def render(self):
        while not self.done and self.feed_channels():
            mixbuff = None
            for chn in self.channels:
                chn.render()
                break

    def feed_channels(self):
        self.time += self.row_length
        p_idx = self.mod.pattern_order[self.pattern]
        self.row += 1
        if self.row >= len(self.mod.patterns[p_idx].rows):
            self.pattern += 1
            self.row = 0
        if self.pattern >= len(self.mod.pattern_order):
            self.loops -= 1
            if not self.loops:
                self.done = True
                return False
            self.pattern = self.mod.restart
        p_idx = self.mod.pattern_order[self.pattern]
        row = self.mod.patterns[p_idx].rows[self.row].notes
        for note, chn in zip(row, self.channels):
            chn.set_note(note)
        return True


class Channel:
    def __init__(self, renderer):
        self.renderer = renderer
        self.volume = 1.
        self.phase = 0
        self.frequency = 0
        self.instrument = self.renderer.mod.instruments[0]

    def set_note(self, note):
        if note.has(xm.NOTE):
            self.frequency = note.frequency()
        else:
            self.frequency = 0
        if note.has(xm.VOLUME):
            self.volume = float(note.volume) / 0x3f
        if note.has(xm.INSTRUMENT):
            idx = note.instrument - 1
            self.instrument = self.renderer.mod.instruments[idx]

    def render(self):
        if self.frequency and self.volume:
            with self.srv.packet() as pkt:
                pkt.add_keyframe(self.idx, int(self.time * 1000))


def play():
    from music import mod
    srv = client.AudioServerInterface()
    with srv.packet() as pkt:
	    pkt.set_input_param(pkt.input_param.audio_engine, 0)
	    pkt.set_input_param(pkt.input_param.mode, pkt.modes.realtime)
    renderer = AcousticAVEModuleRenderer(mod, srv)
    renderer.render()
    with srv.packet() as pkt:
        pkt.output_set_frame(0, 0)


def write_square_wave(length=None, sample_rate=44100):
    if length is None:
        length = sample_rate  # 1s
    period = sample_rate / 220  # A3
    phases = np.arange(length) * np.pi / period
    wav = np.tanh(np.sin(phases) * 10)  # square the sin
    wav = wav / wav.max() * 32000
    wavfile.write(wave_path, sample_rate, wav.astype(np.uint16))



#write_square_wave()


srv = AudioServerInterface()
with srv.packet() as pkt:
    pkt.set_input_param(pkt.input_param.hrtf, pkt.hrtf.cipic)
    pkt.set_input_param(pkt.input_param.audio_engine, 1)
    pkt.set_input_param(pkt.input_param.mode, pkt.modes.iterative)

# add sound sources
with srv.packet() as pkt:
    src = pkt.add_source()
    pkt.set_sound_file(src, wave_path)
    pkt.clear_keyframes(src)
    pkt.add_keyframe(src, 200)
for i in range(10):
    with srv.packet() as pkt:
        pkt.add_keyframe(src, 400 + 100 * i, pos=(float(i) / 2, 0, 0))
#        pkt.add_keyframe(src, 400 + 100 * i, pos=(0, float(i), 0))


with srv.packet() as pkt:
    pkt.output_set_frame(0, 0)
    pkt.output_write_frames(220500)
    pkt.output_iterate()

