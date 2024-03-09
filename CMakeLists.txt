cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# set project name to directory name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# list of source files
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_pio hardware_dma hardware_clocks)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})

file(GLOB_RECURSE PIO_SOURCES "src/*.pio")

# if PIO_SOURCES is not empty
if(PIO_SOURCES)
    pico_generate_pio_header(${PROJECT_NAME} ${PIO_SOURCES})
    target_sources(${PROJECT_NAME} PRIVATE ${PIO_SOURCES})
endif()
