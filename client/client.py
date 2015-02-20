#!/usr/bin/env python
#-*- coding:utf-8 -*-

import re
import socket
from struct import pack

MTU = 1400
PY3 = (bytes != str)

class Object:
    """
        Simple .OBJ loader for the examples.
    """
    def __init__(self, fname):
        with open(fname, 'rt') as obj_file:
            text = obj_file.read()
        lines = [
            line.split("#")[0]  # strip comments
            for line in text.splitlines()
            ]
        self.vertices = [[float(num) for num in re.findall("[0-9-.]+", line)]
                         for line in lines if line.startswith("v ")]
        text = "\n".join(lines)
        materials = re.findall(r"(?s)usemtl\s+(\w+)(.*?)(?=$|usemtl)", text)
        self.geometries = []
        for mat, geometry in materials:
            faces = [[int(num) - 1 for num in re.findall("\d+", line)]
                      for line in geometry.splitlines()
                      if line.startswith("f ")]
            self.geometries.append((faces, mat))


class AudioPacket:
    """
        Context manager for packing configuration changes.
    """
    class cmds:
        listener = 0
        source = 1
        geometry = 2
        reverb = 3
        input_param = 4
        output = 5

    class source_cmds:
        add = 0
        set_pos = 1
        put_audio = 2
        set_sound_file = 3
        start_sound = 4
        clear_keyframes = 5
        add_keyframe = 6

    class geometry_cmds:
        add = 0
        set_geometry = 1
        set_posrot = 2
        set_material = 3
        
    class reverb_cmds:
        add = 0
        area = 1
        volume = 2
        rt60 = 3
        reverb_active = 4
        reverb_mix = 5      

    class input_param:
        mode = 0
        hrtf = 1
        reflections = 2
        frame_rate = 3
        audio_engine = 4

    class modes:
        realtime = 0
        iterative = 1

    class hrtf:
        mit = 1
        cipic = 2
        listen = 3
        tub = 4
        identity = 5

    class output_cmds:
        iterate = 0
        set_frame = 1
        write_frames = 2;

    class audio_engine:
        direct = 0
        aave = 1

    def __init__(self, server):
        self.server = server
        self.cmds_list = []

    # -=-=-=:[ 0: Listener
    def set_listener(self, pos, rot):
        self.cmds_list.append(
            pack("!Bffffff", self.cmds.listener, *(pos + rot))
            )

    # -=-=-=:[ 1: Sources
    def add_source(self):
        self.cmds_list.append(
            pack("!BBB", self.cmds.source, self.source_cmds.add, self.server.source_count)
            )
        retv = self.server.source_count
        self.server.source_count += 1
        return retv

    def set_source_position(self, source, pos):
        self.cmds_list.append(
            pack("!BBBfff", self.cmds.source, self.source_cmds.set_pos,
                 source, *pos)
            )

    def source_put_audio(self, source, samples):
        self.cmds_list.append(
            pack("!BBBB" + "h" * len(samples),
                 self.cmds.source, self.source_cmds.put_audio,
                 source, len(samples), *samples)
            )

    def set_sound_file(self, source, sound_file):
        self.cmds_list.append(
            pack("!BBBB", self.cmds.source, self.source_cmds.set_sound_file,
                 source, len(sound_file)
                 ) + sound_file.encode('latin-1')
            )

    def start_sound(self, source):
        self.cmds_list.append(
            pack("!BBB", self.cmds.source, self.source_cmds.start_sound, source)
            )

    def clear_keyframes(self, source):
        self.cmds_list.append(pack("!BBB", self.cmds.source,
                                   self.source_cmds.clear_keyframes, source))

    def add_keyframe(self, source, time, play=True, pos=None, sound=None,
                     note_ratio=None):
        """
            Add a keyframe to the source at the specified time in milliseconds.
        """
        flags = (1 if play is not None else 0)
        flags |= (2 if pos is not None else 0)
        flags |= (4 if sound is not None else 0)
        flags |= (8 if note_ratio is not None else 0)
        cmd = pack("!BBBBI", self.cmds.source, self.source_cmds.add_keyframe,
                   source, flags, time)
        if pos is not None:
            cmd += pack("!fff", *pos)
        if sound is not None:
            cmd += pack("!B", len(sound)) + sound
        if note_ratio is not None:
            cmd += pack("!f", note_ratio)
        self.cmds_list.append(cmd)

    # -=-=-=:[ 2: Geometries
    def add_geometry(self):
        self.cmds_list.append(
            pack("!BB", self.cmds.geometry, self.geometry_cmds.add)
            )
        retv = self.server.geometry_count
        self.server.geometry_count += 1
        return retv

    def set_geometry(self, geometry, vertices, faces):
        cmd = pack("!BBBBB", self.cmds.geometry,
                   self.geometry_cmds.set_geometry, geometry,
                   len(vertices), len(faces))
        # sequence of <x, y, z> values
        cmd += b''.join(pack("!fff", *vertex) for vertex in vertices)
        # sequence of <# of vertices in face, vertex index 1, ...>
        if PY3:
            cmd += b''.join(bytearray([len(face)] + face) for face in faces)
        else:
            cmd += ''.join(str(bytearray([len(fce)] + fce)) for fce in faces)
        self.cmds_list.append(cmd)

    def set_geometry_posrot(self, geometry, pos, rot):
        self.cmds_list.append(
            pack("!BBBffffff",
                 self.cmds.geometry, self.geometry_cmds.set_posrot,
                 geometry, *(pos + rot))
            )

    def set_geometry_material(self, geometry, material):
        self.cmds_list.append(
            pack("!BBBB", self.cmds.geometry, self.geometry_cmds.set_material,
                 geometry, len(material)
                 ) + material.encode('latin-1')
            )
            
    # -=-=-=:[ 3: Reverb parameters
    def add_reverb(self):
        self.cmds_list.append(
            pack("!BB", self.cmds.reverb, self.reverb_cmds.add)
            )

    def set_reverb_param(self, param_code, param):
        self.cmds_list.append(
            pack("!BBH", self.cmds.reverb, param_code, param)
            )

    # -=-=-=:[ 4: Input parameters
    def set_input_param(self, param_code, param):
        self.cmds_list.append(
            pack("!BBH", self.cmds.input_param, param_code, param)
            )

    # -=-=-=:[ 5: Output
    def output_iterate(self):
        self.cmds_list.append(
            pack("!BB", self.cmds.output, self.output_cmds.iterate)
            )

    def output_set_frame(self, frame, delay):
        self.cmds_list.append(
            pack("!BBii", self.cmds.output, self.output_cmds.set_frame, frame, delay)
            )

    def output_write_frames(self, nframes):
        self.cmds_list.append(
            pack("!BBi", self.cmds.output, self.output_cmds.write_frames, nframes)
            )
			
    def __enter__(self):
        return self

    def __exit__(self, type, val, tb):
        if type is not None or not self.cmds_list:
            return
        packet = b"".join(self.cmds_list)
        if len(packet) > MTU:
            print("Warning: packet bigger than MTU ({} > {})"
                  .format(len(packet), MTU))
        if self.server.debug:
            print("packet ({} bytes):\n* ".format(len(packet)) +
                  "\n* ".join(":".join("%02x" % (c if PY3 else ord(c))
                                       for c in cmd)
                              for cmd in self.cmds_list))
        else:
            self.server.send(packet)


class AudioServerInterface:
    def __init__(self, addr="127.0.0.1", port=34492):
        self.debug = False
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.set_dest(addr, port)
        self.source_count = 0
        self.geometry_count = 0

    def set_dest(self, addr, port):
        self.port = port
        self.dest = addr

    def packet(self):
        return AudioPacket(self)

    def send(self, data):
        print("sending %i bytes"%(len(data)))
        self.sock.sendto(data, (self.dest, self.port))
