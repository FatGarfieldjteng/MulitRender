cmake_minimum_required(VERSION 3.16.1)

project(MultiRender LANGUAGES CXX)

if(MSVC)
	add_compile_options(/MP)
endif()

add_subdirectory(application)