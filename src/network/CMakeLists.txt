
include_directories(${NETWORK_INCLUDE_DIR})
link_directories(${LIBUV_LIB_DIR})

add_library(network tcp_client.cc)

target_link_libraries(network libuv_a.a pthread)