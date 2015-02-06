cmake_minimum_required(VERSION 2.8)

PROJECT(AUDIO_INTERFACE)

SET(CMAKE_CURRENT_SOURCE_DIR /src)
SET(CMAKE_CURRENT_BINARY_DIR /bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_LIBRARY(AAVE_LIBRARY libaave.a "../libaave")

SET(GCC_COMPILE_FLAGS "-std=c++11 -ggdb -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )

add_executable(sys-server src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp src/main_test_udp.cpp)

#add_executable(boost_test src/boost_test.cpp src/kfsys_interface.cpp src/alsa_interface.cpp src/aave_interface.cpp src/test.cpp src/kfsys_sound.cpp src/kfsys_source.cpp src/test.cpp src/util.cpp)

target_link_libraries(sys-server asound ${AAVE_LIBRARY})
#target_link_libraries(boost_test asound ${AAVE_LIBRARY})
