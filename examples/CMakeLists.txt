cmake_minimum_required(VERSION 3.0)

add_executable(image_example 
	${COW_HDRS}
	image_example.cpp
	Image.h
	Image.cpp
)