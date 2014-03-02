cmake_minimum_required(VERSION 2.8)
project(dustbin)
include_directories(${dustbin_SOURCE_DIR}/include)
include_directories(${dustbin_SOURCE_DIR}/recycled/include)
add_subdirectory(recycled)


set(MAIN_SRC_LIST
    src/main.cc
    src/handlers.cc
)

add_executable(dustbin ${MAIN_SRC_LIST})
target_link_libraries(
    dustbin
    boost_filesystem
    boost_program_options
    boost_thread-mt
    boost_regex
    boost_system
    mongoclient
    fcgi
    ctemplate
    recycled
    event
    jsoncpp
)
configure_file(conf/dustbin.conf dustbin.conf)
file(COPY theme DESTINATION .)