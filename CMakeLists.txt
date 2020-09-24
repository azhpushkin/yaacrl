cmake_minimum_required(VERSION 3.0.0)
project(yaacrl VERSION 0.1.0)

include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/src)


add_subdirectory(src)


include_directories(src)
add_executable(yaacrl-test test.cpp)
target_link_libraries(yaacrl-test yaacrl)
