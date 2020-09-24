cmake_minimum_required(VERSION 3.0.0)
project(Yaacrl-lib)

# std::filesystem from C++17 is used
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VENDOR_SOURCES
    vendor/kiss_fft.c
    vendor/MurmurHash3.cpp)
set(SOURCES
    ${VENDOR_SOURCES}
    lib.cpp
    spectrogram.cpp
    fingerprint.cpp)

add_library(yaacrl SHARED ${SOURCES})
target_link_libraries(yaacrl hiredis)
