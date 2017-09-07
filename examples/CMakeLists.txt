cmake_minimum_required(VERSION 2.8.11)

if(NOT HAVE_CXX_REFERENCE_QUALIFIED_FUNCTIONS)
    message("The image_example example relies on reference qualified functions,  which aren't supported by your compiler.")
    return()
endif()


add_executable(image_example 
	${COW_HDRS}
	image_example.cpp
	Image.h
	Image.cpp
)