cmake_minimum_required(VERSION 2.8.11)

project(ImplicitelySharedData CXX C)
set(CMAKE_CXX_STANDARD 11) 

if(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_reference_qualified_functions;")
    set(HAVE_CXX_REFERENCE_QUALIFIED_FUNCTIONS ON)
	add_definitions(-DHAVE_CXX_REFERENCE_QUALIFIED_FUNCTIONS)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(COW_HDRS ${PROJECT_SOURCE_DIR}/include/COW.h)

enable_testing()
add_subdirectory(test)
add_subdirectory(examples)
