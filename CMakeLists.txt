cmake_minimum_required(VERSION 2.8)
project(dustbin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(recycled)
add_subdirectory(plugin)
add_subdirectory(src)

configure_file(conf/dustbin.conf bin/dustbin.conf)
file(COPY admin DESTINATION bin/)