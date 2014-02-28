cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0012 NEW)

project(OpenCTM)

option(BUILD_TOOLSET "Build tools: ctmconv, ctmviewer" ON)
option(BUILD_DOCUMENTATION "Build documentation: manpages" ON)

add_subdirectory(lib)
if(${BUILD_TOOLSET})
	add_subdirectory(tools)
endif()
if(${BUILD_DOCUMENTATION})
	add_subdirectory(doc)
endif()
