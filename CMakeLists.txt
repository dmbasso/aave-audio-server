cmake_minimum_required(VERSION 2.8)

PROJECT(aave-audio-server)

SET(SERVER OFF)
SET(2DVIEW ON)
SET(PYTEST OFF)
SET(BOOST_TEST OFF)

IF(PYTEST)
	SET(SERVER ON)
	SET(PYTHON_TEST_DIR /tests)

	macro(add_python_target tgt)
	  foreach(file ${ARGN})
		set(OUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyo)
		list(APPEND OUT_FILES ${OUT})
		add_custom_command(OUTPUT ${OUT} COMMAND py.test -xv tests)
	  endforeach()

	  add_custom_target(${tgt} ALL DEPENDS ${OUT_FILES})
	endmacro()

	add_python_target(tests ${PYTHON_TEST_DIR}/test_server.py)
ENDIF(PYTEST)

if(SERVER)
	SET(SOURCES src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp src/main_test_udp.cpp)

	FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")

	SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")

	ADD_EXECUTABLE(aave-audio-server ${SOURCES})
	TARGET_LINK_LIBRARIES(aave-audio-server asound ${AAVE_LIBRARY})
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
