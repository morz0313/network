
include_directories(${NETWORK_INCLUDE_DIR})
link_directories(${NETWORK_LIB_DIR})

add_executable(test main.cc)

target_link_libraries(test libnetwork.a libuv_a.a pthread dl)