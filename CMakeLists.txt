cmake_minimum_required(VERSION 2.6)
project(libgreenify)

include(CheckFunctionExists)
check_function_exists(poll HAVE_POLL)
if(NOT HAVE_POLL)
    add_definitions(-D NO_POLL)
endif()

if(DEBUG)
	add_definitions(-D DEBUG)
endif(DEBUG)

add_library(greenify SHARED libgreenify.c)
install(TARGETS greenify DESTINATION lib)
install(FILES libgreenify.h DESTINATION include)
