cmake_minimum_required(VERSION 3.27)
project(wave_simulation)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/raylib-5.0)
add_subdirectory(external/raylib-cpp-5.0.0)

add_executable(wave_simulation src/main.cpp)
target_include_directories(wave_simulation PRIVATE external/thread-pool-4.0.1/include)
target_link_libraries(wave_simulation raylib raylib_cpp)
