cmake_minimum_required(VERSION 3.10)
project(NightWatcher VERSION 1.0 LANGUAGES C)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Debugging compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra" CACHE STRING "Debug C flags" FORCE)

# Include subdirectories
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/sqm-le
    ${PROJECT_SOURCE_DIR}/parser
    ${PROJECT_SOURCE_DIR}/config_file_handler
    ${PROJECT_SOURCE_DIR}/db_handler
    ${PROJECT_SOURCE_DIR}/command_handler
    ${PROJECT_SOURCE_DIR}/weather/AmbientWeather
)

# Gather all source files
file(GLOB NIGHTWATCHER_SOURCES
    ${PROJECT_SOURCE_DIR}/main.c
    ${PROJECT_SOURCE_DIR}/sqm-le/*.c
    ${PROJECT_SOURCE_DIR}/parser/*.c
    ${PROJECT_SOURCE_DIR}/config_file_handler/*.c
    ${PROJECT_SOURCE_DIR}/db_handler/*.c
    ${PROJECT_SOURCE_DIR}/command_handler/*.c
    ${PROJECT_SOURCE_DIR}/weather/AmbientWeather/*.c
)

add_executable(nightwatcher ${NIGHTWATCHER_SOURCES})

target_link_libraries(nightwatcher rrd pthread cjson curl)
