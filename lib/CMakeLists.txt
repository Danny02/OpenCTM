set(liblzma_DIR liblzma)

set(openctm_SOURCES
	openctm.c
	stream.c
	compressRAW.c
	compressMG1.c
	compressMG2.c
)
set(liblzma_SOURCES
	${liblzma_DIR}/Alloc.c
	${liblzma_DIR}/LzFind.c
	${liblzma_DIR}/LzmaDec.c
	${liblzma_DIR}/LzmaEnc.c
	${liblzma_DIR}/LzmaLib.c
)

if(MSVC)
	set(CFLAGS_LZMA /nologo /W3)
	set(DEFINITIONS_LZMA LZMA_PREFIX_CTM)
	set(CFLAGS_CTM ${CFLAGS_LZMA})
	set(DEFINITIONS_CTM ${DEFINITIONS_LZMA} OPENCTM_BUILD _CRT_SECURE_NO_WARNINGS)
	set(CFLAGS_CTM_STATIC ${CFLAGS_CTM})
	set(DEFINITIONS_CTM_STATIC ${DEFINITIONS_CTM})
else()
	set(CFLAGS_LZMA -O3 -W -Wall -std=c99 -pedantic)
	set(DEFINITIONS_LZMA LZMA_PREFIX_CTM)
	set(CFLAGS_CTM ${CFLAGS_LZMA})
	set(DEFINITIONS_CTM ${DEFINITIONS_LZMA} OPENCTM_BUILD)
	set(CFLAGS_CTM_STATIC ${CFLAGS_CTM})
	set(DEFINITIONS_CTM_STATIC ${DEFINITIONS_CTM})
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	list(APPEND CFLAGS_LZMA -fPIC)
	list(APPEND CFLAGS_CTM -fPIC)
	list(APPEND CFLAGS_CTM_STATIC -fPIC)
elseif(APPLE)
	list(APPEND CFLAGS_LZMA -fvisibility=hidden)
	list(APPEND CFLAGS_CTM -fvisibility=hidden)
	list(APPEND CFLAGS_CTM_STATIC -fvisibility=hidden)
endif()

add_library(liblzma OBJECT ${liblzma_SOURCES})
target_compile_definitions(liblzma PUBLIC ${DEFINITIONS_LZMA})
target_compile_options(liblzma PUBLIC ${CFLAGS_LZMA})

include_directories(${liblzma_DIR})

add_library(openctm SHARED $<TARGET_OBJECTS:liblzma> ${openctm_SOURCES})
add_library(openctmstatic STATIC $<TARGET_OBJECTS:liblzma> ${openctm_SOURCES})
target_compile_definitions(openctm PUBLIC ${DEFINITIONS_CTM})
target_compile_definitions(openctmstatic PUBLIC ${DEFINITIONS_CTM_STATIC})
target_compile_options(openctm PUBLIC ${CFLAGS_CTM})
target_compile_options(openctmstatic PUBLIC ${CFLAGS_CTM_STATIC})

if(NOT WIN32)
	target_link_libraries(openctm m)
endif()


install(TARGETS openctm openctmstatic
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES openctm.h openctmpp.h
	DESTINATION include
)
