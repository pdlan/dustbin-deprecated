cmake_minimum_required(VERSION 2.8)
project(dustbin)
include_directories(${dustbin_SOURCE_DIR}/include)
include_directories(${dustbin_SOURCE_DIR}/recycled/include)
include_directories(${dustbin_SOURCE_DIR}/sundown/include)
add_subdirectory(recycled)
add_subdirectory(sundown)
add_subdirectory(plugin)


set(MAIN_SRC_LIST
    src/main.cc
    src/handlers.cc
    src/theme.cc
    src/admin.cc
    src/auth.cc
    src/modifiers.cc
    src/article.cc
    src/setting.cc
    src/plugin.cc
    src/install.cc
)

add_executable(dustbin ${MAIN_SRC_LIST})
target_link_libraries(
    dustbin
    boost_filesystem
    boost_thread-mt
    boost_system
    boost_regex
    mongoclient
    fcgi
    ctemplate
    recycled
    sundown
    event
    jsoncpp
    ssl
)

configure_file(conf/dustbin.conf dustbin.conf)
file(COPY admin DESTINATION .)
