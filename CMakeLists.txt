set(target main.o)

add_executable(${target} main.c)
target_link_libraries(${target} wiringPi)

add_executable(init.o init.c)
target_link_libraries(init.o wiringPi)