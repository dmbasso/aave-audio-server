#!/usr/bin/env python
#-*- coding:utf-8 -*-

import os
import sys
import time
import pytest
import subprocess
from scipy.io import wavfile
from scipy.signal import hilbert
import numpy as np

sys.path.append("client")

import client


SAMPLING_RATE = 44100
wave_path = "square.wav"


def write_square_wave(length=None, sample_rate=None):
    if sample_rate is None:
        sample_rate = SAMPLING_RATE
    if length is None:
        length = sample_rate  # 1s
    period = sample_rate / 220  # A3
    phases = np.arange(length) * np.pi / period
    wav = np.tanh(np.sin(phases) * 10)  # square the sin
    wav = wav / wav.max() * 32000
    wavfile.write(wave_path, sample_rate, wav.astype(np.uint16))


if not os.path.exists(wave_path):
    write_square_wave()


@pytest.fixture(scope="module")
def iface(request):
    """
        The server interface.
    """
    return client.AudioServerInterface()


@pytest.fixture()
def srv(request):
    """
        A server for single tests, so we can check its output.
    """
    proc = subprocess.Popen(
        "./aave-audio-server", stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    time.sleep(.25)
    return proc


def chk_output(substr, srv):
    """
        Wait a bit for the server to process the last command, then
        terminate it, and assert its output contains a substring.
    """
    time.sleep(.25)
    srv.terminate()
    assert substr in srv.communicate()[0]


@pytest.fixture(scope="module")
def srv4all(request):
    """
        A server that is supposed to be kept running during all the tests.
    """
    proc = subprocess.Popen(
        "./aave-audio-server", stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )

    def finalizer():
        proc.terminate()

    request.addfinalizer(finalizer)
    time.sleep(.25)
    return proc


def test_start_uninitialized_source(srv, iface):
    with iface.packet() as pkt:
        pkt.start_sound(0)
    chk_output("necessary to add a source before", srv)


@pytest.mark.parametrize("engine", (0,))
def test_distance_attenuation(srv, iface, engine):
    with iface.packet() as pkt:
        #pkt.set_input_param(pkt.input_param.hrtf, pkt.hrtf.cipic)
        pkt.set_input_param(pkt.input_param.audio_engine, engine)
        pkt.set_input_param(pkt.input_param.mode, pkt.modes.iterative)
        src = pkt.add_source()
        pkt.set_sound_file(src, wave_path)
        pkt.clear_keyframes(src)
    for i in range(10):
        with iface.packet() as pkt:
            # source moving to the right, half a meter every 500 ms
            pkt.add_keyframe(src, 500 * i, pos=(float(i) / 2, 0, 0))
    with iface.packet() as pkt:
        pkt.output_set_frame(0, 0)
        pkt.output_write_frames(10 * SAMPLING_RATE / 2)
        pkt.output_iterate()
    time.sleep(.5)
    srv.terminate()
    print srv.communicate()[0]
    srate, signal = wavfile.read("output.wav")
    assert srate == SAMPLING_RATE
    natural = np.abs(hilbert(signal.astype(float)))
    slen = SAMPLING_RATE / 2
    ref_energy = natural[:slen].sum()
    for i in range(1, 10):
        idx = i * slen
        energy = natural[idx:idx + slen].sum()
        assert np.abs(energy - ref_energy / (i * .5))   # linear decay
