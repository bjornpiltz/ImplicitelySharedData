cmake_minimum_required(VERSION 3.0)

project(ImplicitelySharedData CXX C)
set(CMAKE_CXX_STANDARD 11) 

include_directories(${PROJECT_SOURCE_DIR}/include)

set(COW_HDRS ${PROJECT_SOURCE_DIR}/include/COW.h)

enable_testing()
add_subdirectory(test)
add_subdirectory(examples)
