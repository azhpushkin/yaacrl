cmake_minimum_required(VERSION 3.0.0)
project(yaacrl VERSION 0.1.0)

# std::filesystem from C++17 is used
set(CMAKE_CXX_STANDARD 17)

# Main shared library target
set(VENDOR_SOURCES
    src/vendor/kiss_fft.c
    src/vendor/MurmurHash3.cpp)
set(SOURCES
    ${VENDOR_SOURCES}
    src/yaacrl.cpp
    src/spectrogram.cpp
    src/fingerprint.cpp)

include_directories(yaacrl PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Shared library
add_library(yaacrl SHARED ${SOURCES})
set_property(TARGET yaacrl PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(yaacrl hiredis)


# Static library, compiles with static hiredis library submodule 
add_subdirectory(hiredis)
add_library(yaacrl-static STATIC ${SOURCES})
target_link_libraries(yaacrl hiredis_static)


# Test program target
add_executable(yaacrl-test test.cpp)
target_link_libraries(yaacrl-test yaacrl)
