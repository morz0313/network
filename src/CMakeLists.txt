
set(NETWORK_INCLUDE_DIR ${LIBUV_INCLUDE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/network)
set(NETWORK_LIB_DIR ${LIBUV_LIB_DIR} ${CMAKE_BINARY_DIR}/src/network)


add_subdirectory(network)
add_subdirectory(main)