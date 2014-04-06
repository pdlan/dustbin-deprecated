cmake_minimum_required(VERSION 2.8)
project(markdown)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin/markdown)
add_subdirectory(sundown)
include_directories(${dustbin_SOURCE_DIR}/recycled/include)
include_directories(${dustbin_SOURCE_DIR}/include)
include_directories(${markdown_SOURCE_DIR}/sundown/include)

set(MAIN_SRC_LIST
    main.cc
)

add_library(markdown SHARED ${MAIN_SRC_LIST})

target_link_libraries(
    markdown
    sundown
)
configure_file(plugin.conf ${CMAKE_BINARY_DIR}/bin/plugin/markdown/plugin.conf)