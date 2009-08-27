1. INTRODUCTION
===============

Welcome to OpenCTM!

OpenCTM is a file format and a software library for compressed 3D triangle
meshes. The geometry is compressed to a fraction of comparable file formats
(3DS, STL, VRML...), and the format is easily accessible through a simple,
portable API.

The library is written in portable C (C99), and should compile nicely on any
32/64-bit system regardless of endianity (big endian or little endian).


2. COMPILING
============

There are a few makefiles for different systems and compilers. Just pick the
one that fits your system, and run "make" on the corresponding file. Here are
a few specific instructions:


2.1 Windows, MinGW32
--------------------

mingw32-make -f Makefile.mingw -j2


2.2 Windows, MS Visual Studio (Express) 2008
--------------------------------------------

nmake /f Makefile.msvc


2.3 Mac OS X
------------

make -f Makefile.macosx -j2


2.4 Linux
---------

make -f Makefile.linux -j2


2.5 OpenSolaris
---------------

gmake -f Makefile.linux -j2



3. LICENSE
==========

OpenCTM is released under the zlib/libpng license (see LICENSE.txt).
