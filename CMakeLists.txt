cmake_minimum_required(VERSION 2.8)

PROJECT(aave-audio-server)

OPTION(SERVER "AcousticAVE Audio Server" ON)
OPTION(2DVIEW "2D Audio Visualizer" ON)
OPTION(BOOST_TEST "Boost Test" OFF)
OPTION(WITH_AAVE "Link against libaave" ON)


ADD_CUSTOM_TARGET(tests py.test -xv tests
		  DEPENDS aave-audio-server)

if(SERVER)

	IF(WITH_AAVE)
		ADD_DEFINITIONS(-DWITH_AAVE)
		SET(SOURCES src/kfsys_interface.cpp src/alsa_interface.cpp
			src/aave_interface.cpp src/kfsys_sound.cpp
			src/kfsys_source.cpp src/util.cpp
			src/main.cpp)
		FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")
		ADD_EXECUTABLE(aave-audio-server ${SOURCES})
		TARGET_LINK_LIBRARIES(aave-audio-server asound ${AAVE_LIBRARY})
	ELSE()
		SET(SOURCES src/kfsys_interface.cpp src/alsa_interface.cpp
			src/kfsys_sound.cpp
			src/kfsys_source.cpp src/util.cpp
			src/main.cpp)
		ADD_EXECUTABLE(aave-audio-server ${SOURCES})
		TARGET_LINK_LIBRARIES(aave-audio-server asound)
	ENDIF()

	SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")

endif(SERVER)

if(2DVIEW)
	FIND_PACKAGE(Qt5Widgets REQUIRED)
	SET(CMAKE_AUTOMOC ON)

	SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
	FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")

	SET(SOURCES_2DVIEW 2dview/src/2dview.cpp 2dview/src/view.cpp src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp)

	ADD_EXECUTABLE(2d_view ${SOURCES_2DVIEW})
	TARGET_LINK_LIBRARIES(2d_view ${Qt5Widgets_LIBRARIES} asound ${AAVE_LIBRARY})
ENDIF(2DVIEW)

IF(BOOST_TEST)

	FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")

	SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")

	ADD_EXECUTABLE(boost_test src/boost_test.cpp src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp)

	TARGET_LINK_LIBRARIES(boost_test asound ${AAVE_LIBRARY})

ENDIF(BOOST_TEST)
