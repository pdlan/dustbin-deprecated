cmake_minimum_required(VERSION 2.8)
project(comment)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin/comment)
include_directories(${dustbin_SOURCE_DIR}/recycled/include)
include_directories(${dustbin_SOURCE_DIR}/include)

set(MAIN_SRC_LIST
    main.cc
)

add_library(comment SHARED ${MAIN_SRC_LIST})
configure_file(plugin.conf ${CMAKE_BINARY_DIR}/bin/plugin/comment/plugin.conf)