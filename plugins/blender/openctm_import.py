#!BPY

"""
Name: 'OpenCTM (*.ctm)...'
Blender: 248
Group: 'Import'
Tooltip: 'Import an OpenCTM file'
"""

import bpy
import Blender
from Blender import Mesh, Scene, Window, sys, Image, Draw
import BPyMesh
import BPyAddMesh
import ctypes
from ctypes import *
from ctypes.util import find_library
import os


__author__ = "Marcus Geelnard"
__version__ = "0.1"
__bpydoc__ = """\
This script imports OpenCTM files into Blender. It supports normals,
colours, and texture coordinates per vertex.
"""

# Copyright (C) 2009: Marcus Geelnard
#
# This program is released to the public domain.
#
# Portions of this code are taken from ply_import.py in Blender
# 2.48.
#
# The script uses the OpenCTM shared library (.so, .dll, etc). If no
# such library can be found, the script will exit with an error
# message.
#
# v0.1, 2009-05-31
#    - First test version with an alpha version of the OpenCTM API
#

def file_callback(filename):

	Window.WaitCursor(1)
	try:
		# Load the OpenCTM shared library
		if os.name == 'nt':
			libHDL = WinDLL('openctm.dll')
		else:
			libName = find_library('openctm')
			if not libName:
				Blender.Draw.PupMenu('Could not find the OpenCTM shared library')
				return
			libHDL = CDLL(libName)
		if not libHDL:
			Blender.Draw.PupMenu('Could not open the OpenCTM shared library')
			return

		# Get all the functions from the shared library that we need
		ctmNewContext = libHDL.ctmNewContext
		ctmNewContext.argtypes = [c_int]
		ctmNewContext.restype = c_void_p
		ctmFreeContext = libHDL.ctmFreeContext
		ctmFreeContext.argtypes = [c_void_p]
		ctmGetError = libHDL.ctmGetError
		ctmGetError.argtypes = [c_void_p]
		ctmGetError.restype = c_int
		ctmLoad = libHDL.ctmLoad
		ctmLoad.argtypes = [c_void_p, c_char_p]
		ctmGetInteger = libHDL.ctmGetInteger
		ctmGetInteger.argtypes = [c_void_p, c_int]
		ctmGetInteger.restype = c_int
		ctmGetString = libHDL.ctmGetString
		ctmGetString.argtypes = [c_void_p, c_int]
		ctmGetString.restype = c_char_p
		ctmGetIntegerArray = libHDL.ctmGetIntegerArray
		ctmGetIntegerArray.argtypes = [c_void_p, c_int]
		ctmGetIntegerArray.restype = POINTER(c_int)
		ctmGetFloatArray = libHDL.ctmGetFloatArray
		ctmGetFloatArray.argtypes = [c_void_p, c_int]
		ctmGetFloatArray.restype = POINTER(c_float)
		ctmGetNamedAttribMap = libHDL.ctmGetNamedAttribMap
		ctmGetNamedAttribMap.argtypes = [c_void_p, c_char_p]
		ctmGetNamedAttribMap.restype = c_int

		# Create an OpenCTM context
		ctm = ctmNewContext(0x0101)  # CTM_IMPORT
		try:
			# Load the file
			ctmLoad(ctm, c_char_p(filename))
			err = ctmGetError(ctm)
			if err != 0:
				Blender.Draw.PupMenu('Could not load the file (error code %d)' % err)
				return

			# Get the mesh properties
			vertexCount = ctmGetInteger(ctm, 0x0301)   # CTM_VERTEX_COUNT
			triangleCount = ctmGetInteger(ctm, 0x0302) # CTM_TRIANGLE_COUNT
			hasNormals = ctmGetInteger(ctm, 0x0303)    # CTM_HAS_NORMALS
			texMapCount = ctmGetInteger(ctm, 0x0304)   # CTM_TEX_MAP_COUNT

			# Get indices
			pindices = ctmGetIntegerArray(ctm, 0x0601) # CTM_INDICES

			# Get vertices
			pvertices = ctmGetFloatArray(ctm, 0x0602)  # CTM_VERTICES

			# Get normals
			if hasNormals == 1:
				pnormals = ctmGetFloatArray(ctm, 0x0603) # CTM_NORMALS
			else:
				pnormals = POINTER(c_float)()

			# Get texture coordinates
			if texMapCount > 0:
				ptexCoords = ctmGetFloatArray(ctm, 0x0700) # CTM_TEX_MAP_1
			else:
				ptexCoords = POINTER(c_float)()

			# Get colors
			colorMap = ctmGetNamedAttribMap(ctm, c_char_p('Colors'))
			if colorMap != 0:
				pcolors = ctmGetFloatArray(ctm, colorMap)
			else:
				pcolors = POINTER(c_float)()

			# Create Blender verts and faces
			Vector = Blender.Mathutils.Vector
			verts = []
			for i in range(vertexCount):
				verts.append(Vector(pvertices[i * 3], pvertices[i * 3 + 1], pvertices[i * 3 + 2]))
			faces = []
			for i in range(triangleCount):
				faces.append( (pindices[i * 3], pindices[i * 3 + 1], pindices[i * 3 + 2]) )

			# Create a new Blender mesh as a new object
			objname = Blender.sys.splitext(Blender.sys.basename(filename))[0]
			BPyAddMesh.add_mesh_simple(objname, verts, [], faces)

		finally:
			# Free the OpenCTM context
			ctmFreeContext(ctm)

	finally:
		Window.WaitCursor(0)

	Blender.Redraw()

def main():
	Blender.Window.FileSelector(file_callback, 'Import OpenCTM', '*.ctm')

if __name__=='__main__':
	main()