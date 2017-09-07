cmake_minimum_required(VERSION 2.8.11)

if(NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_reference_qualified_functions;")
    message("The image_example example relies on reference qualified functions,  which aren't supported by your compiler.")
    return()
endif()


add_executable(image_example 
	${COW_HDRS}
	image_example.cpp
	Image.h
	Image.cpp
)