cmake_minimum_required(VERSION 3.20)

project(demo)

set(CMAKE_CXX_STANDARD 14)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} include)

set(SOURCE_FILES
        demo02.cpp 
        include/lockfree_silist.h include/lockfree_list.h
        include/lockfree_binode.h
        include/lockfree_node.h
        include/lockfree_bilist.h
)

add_executable(demo ${SOURCE_FILES})

#add_subdirectory(test)
add_test(NAME TestLinkedList COMMAND utilTest)
enable_testing()
