cmake_minimum_required(VERSION 2.8)
project(comment)
include_directories(../../include)

set(MAIN_SRC_LIST
    main.cc
)

add_library(comment SHARED ${MAIN_SRC_LIST})