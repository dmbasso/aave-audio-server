from xm import FastTracker2Decoder
from binascii import a2b_base64
try:
    from gzip import decompress
except ImportError:
    from gzip import GzipFile
    def decompress(data):
        from StringIO import StringIO
        with GzipFile(fileobj=StringIO(data)) as in_:
            return in_.read()


#gzip -c bdash3.xm | base64
data = b"""\
H4sICDRg11QAA2JkYXNoMy54bQDlU1tPE1EQnukVSIw+8GA0IRsIZNOYKLU2tRdSNm3BIKYPPPFk
dTfBtCmxWIEnTph/wy/xZ/nN7CklIr5AePHs+c7OzJn7znbPfiTjOImD/eN4OkrqwZfj6ShOJkE8
ODkiv17sfxsNzw8mg69D3PiV42UmWsB1BifjWaArIs5kc/lCkf7TtajHM/pMUmKpsmyy1I1osbxn
iVhClhrLW5YGS8XkIDosGyxbdrZZyqYJums6kOyw7LG8Y9k24Qf2gXZ9IAcDh2hOnYdq5eBUOace
buuUNZyTT166NXPYp5m5GrSB1yxvUvMm8NGkss7gNPlNo7eNRlFrbO4aFnLHKO/4FVkkW+oNZ8nO
0As10WveG0VanpMem7I5rP7Bh54PPZ/2q+MlXXbzwkppcmnN6CGK0U8TWbJ7qdSaqy1oXxer79St
Rpjfeccd79iyqNwzSwyOedJ2RjY7Lcux7vW1s22boIb/zppuz3Zku2W7PhurmulX5vpF/LUn36eD
SRKcDn4mN4eXKXyUn6RGRarSE4zEU3pJTUhm6AF94BCIgbHpB7SEzFboOZVBN4Ee0AcOgRgYA2fA
JZArMGcz2Xw+DVb4ew6/brxXid3yHblePEjF/178CDF+A/luaJjpBQAA"""

unpacked = decompress(a2b_base64(data))
mod = FastTracker2Decoder.from_string(unpacked)
