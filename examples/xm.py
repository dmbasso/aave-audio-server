#!/usr/bin/env python
#-*- coding:utf-8 -*-

from struct import unpack


# note flags
PACKED = 0x80
NOTE = 0x01
INSTRUMENT = 0x02
VOLUME = 0x04
EFFECT = 0x08
PARAMETER = 0x10

chromatic_scale = "C- C# D- D# E- F- F# G- G# A- A# B-".split()


class FastTracker2Decoder:
    @classmethod
    def from_file(cls, filepath):
        with open(filepath, 'rb') as in_:
            return cls(in_)

    @classmethod
    def from_string(cls, data):
        try:
            from six import BytesIO
        except ImportError:
            from io import BytesIO
        return cls(BytesIO(data))

    def __init__(self, in_):
        self.in_ = in_
        self.title = "<no module loaded>"
        channels, patterns, instruments = self.header()
        self.patterns = [Pattern(in_, channels) for p in range(patterns)]
        self.instruments = [Instrument(in_) for i in range(instruments)]

    def __repr__(self):
        return "<XM: {}>".format(self.title)

    @staticmethod
    def _dec_str(data):
        return data.strip(b'\0').decode("latin1")

    def header(self):
        data = bytearray(self.in_.read(336))
        if data[:17] != b"Extended Module: " or data[37] != 0x1a:
            raise ValueError("File is not a FastTracker2 module")
        self.title = self._dec_str(data[17:37])
        self.tracker_name = self._dec_str(data[38:58])
        tup = unpack(b"<BBIHHHHHHHH", str(data[58:80]))
        self.format = tup[:2]
        (self.header_size, song_length, self.restart,
         channels, patterns, instruments,
         self.flags, self.tempo, self.bpm) = tup[2:]
        self.ftable = 'amiga' if self.flags & 1 else 'linear'
        if self.header_size != 276:
            raise ValueError("Unexpected header size")
        self.pattern_order = bytearray(data[80:80 + song_length])
        return channels, patterns, instruments


class Pattern:
    def __init__(self, in_, channels):
        header_size, pack_type, rows, data_size = unpack("<IBHH", in_.read(9))
        if header_size != 9:
            raise ValueError("Unexpected pattern header size")
        self.rows = [Row(channels) for row in range(rows)]
        data = bytearray(in_.read(data_size))
        pos = 0
        for row in self.rows:
            pos = row.load(data, pos)


class Row:
    def __init__(self, channels):
        self.notes = [Note() for channel in range(channels)]

    def load(self, data, pos):
        for note in self.notes:
            pos = note.load(data, pos)
        return pos


class Note:
    __slots__ = "info note instrument volume effect parameter".split()

    def load(self, data, pos):
        self.info = data[pos]
        if self.has(PACKED):
            pos += 1
            if self.has(NOTE):
                self.note = data[pos]
                pos += 1
            if self.has(INSTRUMENT):
                self.instrument = data[pos]
                pos += 1
            if self.has(VOLUME):
                self.volume = data[pos]
                pos += 1
            if self.has(EFFECT):
                self.effect = data[pos]
                pos += 1
            if self.has(PARAMETER):
                self.parameter = data[pos]
                pos += 1
        else:
            self.note = self.info
            self.instrument = data[pos + 1]
            self.volume = data[pos + 2]
            self.effect = data[pos + 3]
            self.parameter = data[pos + 4]
            self.info = 0x1f
            pos += 5
        return pos

    def has(self, flag):
        return self.info & flag

    def frequency(self):
        C4 = 261.63
        return C4 * pow(2, (self.note - 48) / 12.)

    def chromatic_note(self):
        if not self.has(NOTE):
            return "..."
        octave, note = divmod(self.note - 1, 12)
        return chromatic_scale[note] + str(octave)

    def __repr__(self):
        retv = []
        retv.append(self.chromatic_note())
        retv.append("%02x" % self.instrument if self.has(INSTRUMENT) else "..")
        retv.append("%02x" % self.volume if self.has(VOLUME) else "..")
        retv.append("%x" % self.effect if self.has(EFFECT) else ".")
        retv.append("%02x" % self.parameter if self.has(PARAMETER) else "..")
        return " ".join(retv)


class Instrument:
    def __init__(self, in_):
        tup = unpack("<I22sBH", in_.read(29))
        header_size, self.name, self.type, waves = tup
        if header_size not in (29, 263):
            raise ValueError("Unexpected instrument header size")
        self.name = FastTracker2Decoder._dec_str(self.name)
        if header_size == 29:
            self.waves = []
            return
        data = in_.read(234)
        wave_header_size = unpack("<I", data[:4])[0]
        self.notes = bytearray(data[4:100])
        self.volume_envelope = bytearray(data[100:148])
        self.panning_envelope = bytearray(data[148:196])
        (self.volume_points, self.panning_points,
         self.volume_sustain_point,
         self.volume_loop_start_point, self.volume_loop_end_point,
         self.panning_sustain_point,
         self.panning_loop_start_point, self.panning_loop_end_point,
         self.volume_type, self.panning_type,
         self.vibrato_type, self.vibrato_sweep, self.vibrato_depth,
         self.vibrato_rate) = bytearray(data[196:210])
        self.volume_fadeout = unpack("<H", data[210:212])[0]
        self.waves = [Wave(in_, wave_header_size) for w in range(waves)]


class Wave:
    def __init__(self, in_, wave_header_size):
        (self.length, self.loop_start, self.loop_end,
         self.volume, self.fine_tune, self.flags,
         self.panning, self.relative_note, self.reserved,
         self.name) = unpack("<IIIBBBBBB22s", in_.read(wave_header_size))
        self.name = FastTracker2Decoder._dec_str(self.name)
        self.data = in_.read(self.length)
