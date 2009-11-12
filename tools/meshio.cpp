//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        meshio.cpp
// Description: Mesh I/O using different file format loaders/savers.
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
#include <string>
#include "mesh.h"
#include "convoptions.h"
#include "ctm.h"
#include "ply.h"
#include "stl.h"
#include "3ds.h"
#include "dae.h"

using namespace std;


// Convert a string to upper case.
static string UpperCase(const string &aString)
{
  string result(aString);
  for(unsigned int i = 0; i < result.size(); ++ i)
    result[i] = toupper(result[i]);
  return result;
}

// Extract the file extension of a file name.
static string ExtractFileExt(const string &aString)
{
  string result = "";
  size_t extPos = aString.rfind(".");
  if(extPos != string::npos)
    result = aString.substr(extPos);
  return result;
}

/// Import a mesh from a file.
void ImportMesh(const char * aFileName, Mesh &aMesh)
{
  string fileExt = UpperCase(ExtractFileExt(string(aFileName)));
  if(fileExt == string(".PLY"))
    Import_PLY(aFileName, aMesh);
  else if(fileExt == string(".STL"))
    Import_STL(aFileName, aMesh);
  else if(fileExt == string(".3DS"))
    Import_3DS(aFileName, aMesh);
  else if(fileExt == string(".DAE"))
    Import_DAE(aFileName, aMesh);
  else if(fileExt == string(".CTM"))
    Import_CTM(aFileName, aMesh);
  else
    throw runtime_error("Unknown input file extension.");
}

/// Export a mesh to a file.
void ExportMesh(const char * aFileName, Mesh &aMesh, Options &aOptions)
{
  string fileExt = UpperCase(ExtractFileExt(string(aFileName)));
  if(fileExt == string(".PLY"))
    Export_PLY(aFileName, aMesh);
  else if(fileExt == string(".STL"))
    Export_STL(aFileName, aMesh);
  else if(fileExt == string(".3DS"))
    Export_3DS(aFileName, aMesh);
  else if(fileExt == string(".DAE"))
    Export_DAE(aFileName, aMesh);
  else if(fileExt == string(".CTM"))
    Export_CTM(aFileName, aMesh, aOptions);
  else
    throw runtime_error("Unknown output file extension.");
}
