cmake_minimum_required(VERSION 3.2)
project(worm)

include_directories("3rdParty/sprout")
include_directories("3rdParty/stmp")
include_directories(include)

list(APPEND WORM_EXAMPLES_DEFAULT_CXX_FLAGS -std=c++1y -Wall -pedantic)

function(add_example_target filename)
    get_filename_component(NAME ${filename} NAME_WE)
    add_executable(worm.examples.${NAME} ${filename})

    target_compile_options(worm.examples.${NAME} PUBLIC ${WORM_EXAMPLES_DEFAULT_CXX_FLAGS})
endfunction()

add_subdirectory(examples)

