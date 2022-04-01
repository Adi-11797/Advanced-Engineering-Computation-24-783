cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ps5_1)
add_subdirectory(ps5_2)
add_subdirectory(ps5_3)
add_subdirectory(bintreelib)

add_subdirectory(../../public/src ${CMAKE_BINARY_DIR}/public)
add_subdirectory(../../MMLPlayer/ym2612 ${CMAKE_BINARY_DIR}/ym2612)
add_subdirectory(../../MMLPlayer/mmlplayer ${CMAKE_BINARY_DIR}/mmlplayer)

enable_testing()