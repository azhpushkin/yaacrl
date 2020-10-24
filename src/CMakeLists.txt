cmake_minimum_required(VERSION 3.0.0)
project(Yaacrl-lib)

set(VENDOR_SOURCES
    vendor/kiss_fft.c
    vendor/MurmurHash3.cpp)
set(SOURCES
    ${VENDOR_SOURCES}
    yaacrl.cpp
    spectrogram.cpp
    fingerprint.cpp)

add_library(yaacrl SHARED ${SOURCES})
target_link_libraries(yaacrl hiredis)
