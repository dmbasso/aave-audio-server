CC=g++
CFLAGS = -std=c++11 -ggdb #-Wall
CXXFLAGS = -std=c++11 -ggdb -fPIC #-Wall
INCLUDES =
LDFLAGS = -L../libaave
LIBS = -laave -lasound
SRCS = kfsys_sound.cpp kfsys_source.cpp kfsys_interface.cpp aave_interface.cpp alsa_interface.cpp test.cpp main_test.cpp util.cpp
OBJS = $(SRCS:.cpp=.o)
MAIN = sys-server
TLIB_OBJS = kfsys_sound.o kfsys_source.o kfsys_interface.o aave_interface.o alsa_interface.o test.o

.PHONY: depend clean

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LDFLAGS) $(LIBS)

_testlib.so: $(TLIB_OBJS)
	g++ -shared -fPIC -o _testlib.so $(TLIB_OBJS) $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
