cmake_minimum_required(VERSION 3.5)
project(nwconsole C)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Enable debug symbols for Debug builds
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

set(CMAKE_C_STANDARD 99)

add_executable(nwconsole
    nwconsole.c
)

target_link_libraries(nwconsole ncurses)
