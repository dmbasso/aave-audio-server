# AcousticAVE auralisation library
LIBS += ../../libaave/libaave.a -lasound
QMAKE_CXXFLAGS += -std=c++11

#QT += widgets

HEADERS += ../src/kfsys_interface.h ../src/alsa_interface.h ../src/aave_interface.h ../src/test.h ../src/kfsys_sound.h ../src/kfsys_source.h ../src/test.h ../src/util.h src/view.h
SOURCES += ../src/kfsys_interface.cpp ../src/alsa_interface.cpp ../src/aave_interface.cpp ../src/test.cpp ../src/kfsys_sound.cpp ../src/kfsys_source.cpp ../src/util.cpp src/2dview.cpp src/view.cpp
