OpenCTM for Maya
============
Exports Maya models to OpenCTM format and viceversa. Currently supports importing and exporting the following:

- Vertices
- Normals
- UV sets (Partially. Only one uv per vertex)
- Vertex colors (Import only)

INSTRUCTIONS
============

The OpenCTM Maya export scripts makes it possible to export OpenCTM format files in Maya (http://autodesk.com/).

To use these scripts, they need to be properly installed along with the OpenCTM shared library.


Windows:
--------
1) Copy the `scripts` and `plug-ins` folders to the appropriate maya folder (e.g. `C:\Users\username\Documents\maya\2018` for Maya 2018).
2) Copy `openctm.dll` to the maya plugin folder (e.g. `C:\Users\username\Documents\maya\2018\plug-ins` for Maya 2018).
3) Copy `openctm.py` in `/bindings/python/` to the maya plugin folder (e.g. `C:\Users\username\Documents\maya\2018\plug-ins` for Maya 2018).

Mac OS X:
---------
1) Copy the `scripts` and `plug-ins` folders to the appropriate maya folder (e.g. `/Users/Shared/Autodesk/maya/2018` for Maya 2018).
2) Copy the file `libopenctm.dylib` to `/usr/local/lib` (e.g. using "`sudo cp libopenctm.dylib /usr/local/lib/`").
3) Copy openctm.py in `/bindings/python/` to the maya plugin folder (e.g. `/Users/Shared/Autodesk/maya/2018/plug-ins` for Maya 2018).

Linux:
------

1) Copy the file "libopenctm.so" to `/usr/lib` (e.g. using `sudo cp libopenctm.so /usr/lib/`).

Activate the plug-in:
--------------------

After that, you need to activate the plugin. In Maya, open `Window > Settings/Preferences > Plug-in Manager` and enable the checkboxes next to `OpenCTMTranslator.py`.


USAGE
=====
Use the regular Export menus within Maya, select `ctm`.
