project(GPIO_control C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(GPIO_control
        gpio.c
        gpio.h
        main.c)
