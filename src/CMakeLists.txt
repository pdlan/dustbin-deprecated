cmake_minimum_required(VERSION 2.8)
include_directories(${dustbin_SOURCE_DIR}/include)
include_directories(${dustbin_SOURCE_DIR}/recycled/include)

set(MAIN_SRC_LIST
    main.cc
    handlers.cc
    theme.cc
    admin.cc
    auth.cc
    modifiers.cc
    article.cc
    setting.cc
    plugin.cc
    install.cc
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
    event
    jsoncpp
    ssl
)