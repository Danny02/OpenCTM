#!BPY

"""
Name: 'OpenCTM (*.ctm)...'
Blender: 248
Group: 'Export'
Tooltip: 'Export active object to OpenCTM (compressed) format'
"""

import bpy
import Blender
from Blender import Mesh, Scene, Window, sys, Image, Draw
import BPyMesh
import ctypes
from ctypes import *
from ctypes.util import find_library
import os


__author__ = "Marcus Geelnard"
__version__ = "0.1"
__bpydoc__ = """\
This script exports OpenCTM files from Blender. It supports normals,
colours, and texture coordinates per vertex. Only one mesh can be exported
at a time.
"""

# Copyright (C) 2009: Marcus Geelnard
#
# This program is released to the public domain.
#
# The script uses the OpenCTM shared library (.so, .dll, etc). If no
# such library can be found, the script will exit with an error
# message.
#
# v0.1, 2009-05-25
#    - First test version with an alpha version of the OpenCTM API
#


def file_callback(filename):
	
	if not filename.lower().endswith('.ctm'):
		filename += '.ctm'

	# Get object mesh from the selected object
	scn = bpy.data.scenes.active
	ob = scn.objects.active
	if not ob:
		Blender.Draw.PupMenu('Error%t|Select 1 active object')
		return
	mesh = BPyMesh.getMeshFromObject(ob, None, False, False, scn)
	if not mesh:
		Blender.Draw.PupMenu('Error%t|Could not get mesh data from active object')
		return

	# Check which mesh properties are present...
	hasVertexUV = mesh.vertexUV
	hasVertexColors = mesh.vertexColors

	# Show a GUI for the export settings
	pupBlock = []
	EXPORT_APPLY_MODIFIERS = Draw.Create(1)
	pupBlock.append(('Apply Modifiers', EXPORT_APPLY_MODIFIERS, 'Use transformed mesh data.'))
	EXPORT_NORMALS = Draw.Create(1)
	pupBlock.append(('Normals', EXPORT_NORMALS, 'Export vertex normal data.'))
	if hasVertexUV:
		EXPORT_UV = Draw.Create(1)
		pupBlock.append(('UVs', EXPORT_UV, 'Export texface UV coords.'))
	if hasVertexColors:
		EXPORT_COLORS = Draw.Create(1)
		pupBlock.append(('Colors', EXPORT_COLORS, 'Export vertex Colors.'))
	if not Draw.PupBlock('Export...', pupBlock):
		return

	# Adjust export settings according to GUI selections
	EXPORT_APPLY_MODIFIERS = EXPORT_APPLY_MODIFIERS.val
	EXPORT_NORMALS = EXPORT_NORMALS.val
	if hasVertexUV:
		EXPORT_UV = EXPORT_UV.val
	else:
		EXPORT_UV = False
	if hasVertexColors:
		EXPORT_COLORS = EXPORT_COLORS.val
	else:
		EXPORT_COLORS = False

	is_editmode = Blender.Window.EditMode()
	if is_editmode:
		Blender.Window.EditMode(0, '', 0)
	Window.WaitCursor(1)
	try:
		# Get the mesh, again, this time with/without modifiers (from GUI selection)
		mesh = BPyMesh.getMeshFromObject(ob, None, EXPORT_APPLY_MODIFIERS, False, scn)
		if not mesh:
			Blender.Draw.PupMenu('Error%t|Could not get mesh data from active object')
			return
		mesh.transform(ob.matrixWorld)

		# Count triangles (quads count as two triangles)
		triangleCount = 0
		for f in mesh.faces:
			if len(f.v) == 4:
				triangleCount += 2
			else:
				triangleCount += 1

		# Extract indices from the Blender mesh (quads are split into two triangles)
		pindices = cast((c_int * 3 * triangleCount)(), POINTER(c_int))
		i = 0
		for f in mesh.faces:
			pindices[i * 3] = c_int(f.v[0].index)
			pindices[i * 3 + 1] = c_int(f.v[1].index)
			pindices[i * 3 + 2] = c_int(f.v[2].index)
			i += 1
			if len(f.v) == 4:
				pindices[i * 3] = c_int(f.v[0].index)
				pindices[i * 3 + 1] = c_int(f.v[2].index)
				pindices[i * 3 + 2] = c_int(f.v[3].index)
				i += 1

		# Extract vertex array from the Blender mesh
		vertexCount = len(mesh.verts)
		pvertices = cast((c_float * 3 * vertexCount)(), POINTER(c_float))
		i = 0
		for v in mesh.verts:
			pvertices[i * 3] = c_float(v.co.x)
			pvertices[i * 3 + 1] = c_float(v.co.y)
			pvertices[i * 3 + 2] = c_float(v.co.z)
			i += 1

		# Extract normals
		if EXPORT_NORMALS:
			pnormals = cast((c_float * 3 * vertexCount)(), POINTER(c_float))
			i = 0
			for v in mesh.verts:
				pnormals[i * 3] = c_float(v.no.x)
				pnormals[i * 3 + 1] = c_float(v.no.y)
				pnormals[i * 3 + 2] = c_float(v.no.z)
				i += 1
		else:
			pnormals = POINTER(c_float)()

		# Extract UVs
		if EXPORT_UV:
			ptexCoords = cast((c_float * 2 * vertexCount)(), POINTER(c_float))
			i = 0
			for v in mesh.verts:
				ptexCoords[i * 2] = c_float(v.uvco[0])
				ptexCoords[i * 2 + 1] = c_float(v.uvco[1])
				i += 1
		else:
			ptexCoords = POINTER(c_float)()

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
		ctmDefineMesh = libHDL.ctmDefineMesh
		ctmDefineMesh.argtypes = [c_void_p, POINTER(c_float), c_int, POINTER(c_int), c_int, POINTER(c_float)]
		ctmSave = libHDL.ctmSave
		ctmSave.argtypes = [c_void_p, c_char_p]

		# Create an OpenCTM context
		ctm = ctmNewContext(0x0102)
		try:
			# Define the mesh
			ctmDefineMesh(ctm, pvertices, c_int(vertexCount), pindices, c_int(triangleCount), pnormals)

			# Save the file
			ctmSave(ctm, c_char_p(filename))
		finally:
			# Free the OpenCTM context
			ctmFreeContext(ctm)

	finally:
		Window.WaitCursor(0)
		if is_editmode:
			Blender.Window.EditMode(1, '', 0)

def main():
	Blender.Window.FileSelector(file_callback, 'Export OpenCTM', Blender.sys.makename(ext='.ctm'))

if __name__=='__main__':
	main()