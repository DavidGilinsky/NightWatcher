cmake_minimum_required(VERSION 3.5)
project(nwconsole C)

set(CMAKE_C_STANDARD 99)

add_executable(nwconsole
    nwconsole.c
)

target_link_libraries(nwconsole ncurses)
