cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

project(utilTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        lockfree_list_concurrent_test.cpp lockfree_list_normal_test.cpp test_linkedlist.h
        ../include/lockfree_silist.h ../include/lockfree_list.h
        ../include/lockfree_binode.h
        ../include/lockfree_node.h
        ../include/lockfree_bilist.h
)

find_package(TBB REQUIRED)
find_path(CATCH_INCLUDE_DIR NAMES catch.hpp PATH_SUFFIXES catch2)

add_executable(utilTest ${SOURCE_FILES})

target_include_directories(utilTest PRIVATE ../include ${CATCH_INCLUDE_DIR})
target_link_libraries(utilTest onetbb::onetbb)

add_test(NAME TestLinkedList COMMAND utilTest)
enable_testing()
