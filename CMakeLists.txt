cmake_minimum_required(VERSION 3.10)
project(week04)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE OpenGL::GL glfw)
