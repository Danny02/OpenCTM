#!BPY

"""
Name: 'OpenCTM export (*.ctm)...'
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
colours, and texture coordinates per vertex.
Only one mesh can be exported at a time.
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


def rvec3d(v):	return round(v[0], 6), round(v[1], 6), round(v[2], 6)
def rvec2d(v):	return round(v[0], 6), round(v[1], 6)

def file_callback(filename):
	
	if not filename.lower().endswith('.ctm'):
		filename += '.ctm'
	
	scn= bpy.data.scenes.active
	ob= scn.objects.active
	if not ob:
		Blender.Draw.PupMenu('Error%t|Select 1 active object')
		return

	EXPORT_APPLY_MODIFIERS = Draw.Create(1)
	EXPORT_NORMALS = Draw.Create(1)
	EXPORT_UV = Draw.Create(1)
	EXPORT_COLORS = Draw.Create(1)
	
	pup_block = [\
	('Apply Modifiers', EXPORT_APPLY_MODIFIERS, 'Use transformed mesh data.'),\
	('Normals', EXPORT_NORMALS, 'Export vertex normal data.'),\
	('UVs', EXPORT_UV, 'Export texface UV coords.'),\
	('Colors', EXPORT_COLORS, 'Export vertex Colors.'),\
	]
	
	if not Draw.PupBlock('Export...', pup_block):
		return
	
	is_editmode = Blender.Window.EditMode()
	if is_editmode:
		Blender.Window.EditMode(0, '', 0)
	
	Window.WaitCursor(1)
	
	EXPORT_APPLY_MODIFIERS = EXPORT_APPLY_MODIFIERS.val
	EXPORT_NORMALS = EXPORT_NORMALS.val
	EXPORT_UV = EXPORT_UV.val
	EXPORT_COLORS = EXPORT_COLORS.val
	
	mesh = BPyMesh.getMeshFromObject(ob, None, EXPORT_APPLY_MODIFIERS, False, scn)
	
	if not mesh:
		Blender.Draw.PupMenu('Error%t|Could not get mesh data from active object')
		return
	
	mesh.transform(ob.matrixWorld)

	vertexUV = mesh.vertexUV
	vertexColors = mesh.vertexColors
	
	if not vertexUV:						EXPORT_UV = False
	if not vertexColors:					EXPORT_COLORS = False

	if not EXPORT_UV:						faceUV = vertexUV = False
	if not EXPORT_COLORS:					vertexColors = False
	
	# incase
	color = uvcoord = uvcoord_key = normal = normal_key = None

	# Load the OpenCTM shared library
	libName = find_library('openctm');
	if not libName:
		Blender.Draw.PupMenu('Could not find the OpenCTM shared library')
		return
	if os.name == 'nt':
		openctm = WinDLL(libName);
	else:
		openctm = CDLL(libName);
	if not openctm:
		Blender.Draw.PupMenu('Could not open the OpenCTM shared library')
		return

	# Create an OpenCTM context
	ctm = openctm.ctmNewContext(0x0102);

	# Do stuff....
	# ....
	# ....

	# Free the OpenCTM context
	openctm.ctmFreeContext(ctm);

	if is_editmode:
		Blender.Window.EditMode(1, '', 0)

def main():
	Blender.Window.FileSelector(file_callback, 'OpenCTM Export', Blender.sys.makename(ext='.ctm'))

if __name__=='__main__':
	main()