cmake_minimum_required(VERSION 3.0.0)
project(yaacrl VERSION 0.1.0)

# std::filesystem from C++17 is used
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


# Main shared library target
set(VENDOR_SOURCES
    src/vendor/kiss_fft.c
    src/vendor/MurmurHash3.cpp
    src/vendor/sqlite3.c)
set(SOURCES
    ${VENDOR_SOURCES}
    src/files.cpp
    src/yaacrl.cpp
    src/config.cpp
    src/spectrogram.cpp
    src/fingerprint.cpp)

include_directories(yaacrl PUBLIC ${PROJECT_SOURCE_DIR}/include/yaacrl)
include_directories(yaacrl PUBLIC ${PROJECT_SOURCE_DIR}/src)  # vendor headers

# Shared library
add_library(yaacrl SHARED ${SOURCES})
target_link_libraries(yaacrl dl)
target_link_libraries(yaacrl pthread)


# Static library, needs manual linking with ldl and pthreads
add_library(yaacrl-static STATIC ${SOURCES})
target_link_libraries(yaacrl-static INTERFACE dl)
target_link_libraries(yaacrl-static INTERFACE pthread)


# Test programs target
add_executable(yaacrl-test test.cpp)
target_link_libraries(yaacrl-test yaacrl-static)

