cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(target main.o)

add_executable(${target} main.cpp)
target_link_libraries(${target} wiringPi m ${OpenCV_LIBS})

add_executable(init.o init.c)
target_link_libraries(init.o wiringPi)