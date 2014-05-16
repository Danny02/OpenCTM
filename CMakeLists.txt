cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0012 NEW)

# Workaround target_compile_options for cmake < 2.8.12
if(${CMAKE_VERSION} VERSION_LESS "2.8.12")
	function(target_compile_options target scope)
		set(args ${ARGV})
		list(REMOVE_AT args 0 1)
		string(REPLACE ";" " " CFLAGS_STRING "${args}")
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${CFLAGS_STRING}")
	endfunction()
endif()


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
