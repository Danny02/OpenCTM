//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        obj.cpp
// Description: Implementation of the OBJ file format importer/exporter.
//-----------------------------------------------------------------------------
// Copyright (c) 2009 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include "obj.h"

#ifdef _MSC_VER
typedef unsigned int uint32;
#else
#include <stdint.h>
typedef uint32_t uint32;
#endif

using namespace std;


/// Import a mesh from an OBJ file.
void Import_OBJ(const char * aFileName, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();

  // Open the input file
  ifstream f(aFileName, ios_base::in | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open input file.");

  // ...
  throw runtime_error("OBJ file import is not yet implemented.");

  // Close the input file
  f.close();
}

/// Export a mesh to an OBJ file.
void Export_OBJ(const char * aFileName, Mesh &aMesh)
{
  // Open the output file
  ofstream f(aFileName, ios_base::out | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open output file.");

  // Write comment
  f << "# " << aMesh.mComment << endl;
  f << "# Generator: ctmconv" << endl;

  // ...
  throw runtime_error("OBJ file export is not yet implemented.");

  // Close the output file
  f.close();
}
