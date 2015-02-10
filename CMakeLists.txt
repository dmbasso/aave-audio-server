cmake_minimum_required(VERSION 2.8)

PROJECT(aave-audio-server)

SET(SOURCES src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp src/main_test_udp.cpp)

FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")

SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} asound ${AAVE_LIBRARY})

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

#add_executable(boost_test src/boost_test.cpp src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp)

#target_link_libraries(boost_test asound ${AAVE_LIBRARY})
