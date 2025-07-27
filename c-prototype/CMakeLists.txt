cmake_minimum_required(VERSION 3.10)
project(MultiPassWPA3)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pg")

include_directories(src)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(bench)

#add_subdirectory(lib/googletest)
