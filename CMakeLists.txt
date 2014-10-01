cmake_minimum_required(VERSION 2.8)
project(vtkOpenCTM)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

INCLUDE_DIRECTORIES(lib)
INCLUDE_DIRECTORIES(lib/liblzma)
INCLUDE_DIRECTORIES(tools/rply)
INCLUDE_DIRECTORIES(tools/tinyxml)

AUX_SOURCE_DIRECTORY(lib ctm) 
AUX_SOURCE_DIRECTORY(lib/liblzma lzma) 
AUX_SOURCE_DIRECTORY(tools/rply rply) 
AUX_SOURCE_DIRECTORY(tools/tinyxml tinyxml)

ADD_DEFINITIONS (-D VTKINCLUDED)

ADD_EXECUTABLE(mesh2ctm 
	tools/ctmconv.cpp
	tools/convoptions.cpp
	tools/systimer.cpp
	tools/mesh.cpp
	tools/meshio.cpp
	tools/ctm.cpp
	tools/meshio.cpp
	tools/ply.cpp
	tools/stl.cpp
	tools/3ds.cpp
	tools/dae.cpp
	tools/lwo.cpp
	tools/obj.cpp
	tools/off.cpp
	tools/wrl.cpp
	tools/common.cpp
	${ctm} ${lzma} ${rply} ${tinyxml}
)
TARGET_LINK_LIBRARIES(mesh2ctm vtkHybrid)

