cmake_minimum_required(VERSION 3.10)
project(shader)
set(CMAKE_CXX_STANDARD 17)

set(GLAD_DIR "D:/lib/glad")
set(OpenGL_DIR "D:/lib/glfw")
find_package(OpenGL REQUIRED)
include_directories(${GLAD_DIR}/include ${OpenGL_DIR}/include)
# message(STATUS ${GLAD_DIR} ${OpenGL_DIR})

add_executable(shader main.cpp ${GLAD_DIR}/src/glad.c)
target_link_libraries(shader ${OpenGL_DIR}/lib/glfw3.dll)

