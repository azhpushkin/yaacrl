cmake_minimum_required(VERSION 3.0.0)
project(yaacrl VERSION 0.1.0)

# std::filesystem from C++17 is used
set(CMAKE_CXX_STANDARD 17)


include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/src)


add_subdirectory(src)


include_directories(src)
add_executable(yaacrl-test test.cpp)
target_link_libraries(yaacrl-test yaacrl)
