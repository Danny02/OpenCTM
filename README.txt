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

nmake /fMakefile.msvc (not yet supported!)


2.3 Linux
---------

make -f Makefile.linux -j2


2.4 Mac OS X
------------

make -f Makefile.macosx -j2


3. LICENSE
==========

OpenCTM is released under the zlib/libpng license, which reads as follows:

Copyright (c) 2009 Marcus Geelnard

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
