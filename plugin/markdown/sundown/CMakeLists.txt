cmake_minimum_required(VERSION 2.8)
project(sundown)
include_directories(${sundown_SOURCE_DIR}/include)

set(MAIN_SRC_LIST
    src/autolink.c
    src/buffer.c
    src/markdown.c
    src/stack.c
    html/html.c
    html/html_smartypants.c
    html/houdini_href_e.c
    html/houdini_html_e.c
)

add_library(sundown STATIC ${MAIN_SRC_LIST})