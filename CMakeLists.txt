cmake_minimum_required(VERSION 3.10)
project(shader)
set(CMAKE_CXX_STANDARD 17)

set(GLAD_DIR "D:/lib/glad")
set(GLFW_DIR "D:/lib/glfw")
find_package(OpenGL REQUIRED)
include_directories(${GLAD_DIR}/include ${GLFW_DIR}/include)
add_executable(shader main.cpp src/glad.c)
link_directories(${GLFW_DIR}/lib)
target_link_libraries(shader libglfw3dll.a)

