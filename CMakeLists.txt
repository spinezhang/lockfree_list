cmake_minimum_required(VERSION 3.20)

project(utils)

set(CMAKE_CXX_STANDARD 11)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} include)

# add_library(utils
#         src/LockFreeBiList.cpp include/lockfreebilist.h
# )

add_subdirectory(test)
add_test(NAME TestLinkedList COMMAND utilTest)
enable_testing()
