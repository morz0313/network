cmake_minimum_required(VERSION 3.0.0)
project(osp_studio VERSION 0.1.0)

set(LIBUV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libuv/include)
set(LIBUV_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libuv)

add_subdirectory(src)