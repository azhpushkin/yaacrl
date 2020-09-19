cmake_minimum_required(VERSION 3.0.0)


link_libraries(hiredis)

set(DIR_PATH ${CMAKE_BINARY_DIR})
set(SOURCES
    lib.cpp
    spectrogram.cpp
    fingerprint.cpp
    MurmurHash3.cpp
    kissfft/kiss_fft.c
)

add_executable(yaacrl-main ${SOURCES} main.cpp)
add_library(yaacrl SHARED ${SOURCES})
