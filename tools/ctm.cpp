//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        ctm.h
// Description: Implementation of the OpenCTM file format importer/exporter.
//-----------------------------------------------------------------------------
// Copyright (c) 2009-2010 Marcus Geelnard
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
#include <openctm2.h>
#include "ctm.h"

using namespace std;


/// Import an OpenCTM file from a file.
void Import_CTM(const char * aFileName, Mesh * aMesh)
{
  // Clear the mesh
  aMesh->Clear();

  // Load the file using the OpenCTM API
  CTMimporter ctm;

  // Load the file header
  ctm.OpenReadFile(aFileName);

  // Extract file comment
  const char * comment = ctm.GetString(CTM_FILE_COMMENT);
  if(comment)
    aMesh->mComment = string(comment);

  // Allocate memory for the indices
  CTMuint numTriangles = ctm.GetInteger(CTM_TRIANGLE_COUNT);
  aMesh->mIndices.resize(numTriangles * 3);
  ctm.ArrayPointer(CTM_INDICES, 3, CTM_UINT, 0, &aMesh->mIndices[0]);

  // Allocate memory for the vertices
  CTMuint numVertices = ctm.GetInteger(CTM_VERTEX_COUNT);
  aMesh->mVertices.resize(numVertices);
  ctm.ArrayPointer(CTM_VERTICES, 3, CTM_FLOAT, 0, &aMesh->mVertices[0].x);

  // Allocate memory for the normals
  if(ctm.GetBoolean(CTM_HAS_NORMALS) == CTM_TRUE)
  {
    aMesh->mNormals.resize(numVertices);
    ctm.ArrayPointer(CTM_NORMALS, 3, CTM_FLOAT, 0, &aMesh->mNormals[0].x);
  }

  // Allocate memory for the texture coordinates
  if(ctm.GetInteger(CTM_UV_MAP_COUNT) > 0)
  {
    aMesh->mTexCoords.resize(numVertices);
    ctm.ArrayPointer(CTM_UV_MAP_1, 2, CTM_FLOAT, 0, &aMesh->mTexCoords[0].u);
    const char * str = ctm.GetUVMapString(CTM_UV_MAP_1, CTM_FILE_NAME);
    if(str)
      aMesh->mTexFileName = string(str);
    else
      aMesh->mTexFileName = string("");
  }

  // Allocate memory for the vertex colors
  CTMenum colorAttrib = ctm.GetNamedAttribMap("Color");
  if(colorAttrib != CTM_NONE)
  {
    aMesh->mColors.resize(numVertices);
    ctm.ArrayPointer(colorAttrib, 4, CTM_FLOAT, 0, &aMesh->mColors[0].x);
  }

  // Load the mesh data
  ctm.ReadMesh();
}

/// Export an OpenCTM file to a file.
void Export_CTM(const char * aFileName, Mesh * aMesh, Options &aOptions)
{
  // Save the file using the OpenCTM API
  CTMexporter ctm;

  // Define mesh
  ctm.TriangleCount(aMesh->mIndices.size() / 3);
  ctm.VertexCount(aMesh->mVertices.size());
  ctm.ArrayPointer(CTM_INDICES, 3, CTM_UINT, 0, &aMesh->mIndices[0]);
  ctm.ArrayPointer(CTM_VERTICES, 3, CTM_FLOAT, 0, &aMesh->mVertices[0].x);
  if(aMesh->HasNormals() && !aOptions.mNoNormals)
    ctm.ArrayPointer(CTM_NORMALS, 3, CTM_FLOAT, 0, &aMesh->mNormals[0].x);
  if(aMesh->HasTexCoords())
  {
    const char * fileName = NULL;
    if(aMesh->mTexFileName.size() > 0)
      fileName = aMesh->mTexFileName.c_str();
    CTMenum map = ctm.AddUVMap("TexCoords", fileName);
    ctm.ArrayPointer(map, 2, CTM_FLOAT, 0, &aMesh->mTexCoords[0].u);
    ctm.UVCoordPrecision(map, aOptions.mTexMapPrecision);
  }
  if(aMesh->HasColors())
  {
    CTMenum map = ctm.AddAttribMap("Color");
    ctm.ArrayPointer(map, 4, CTM_FLOAT, 0, &aMesh->mColors[0].x);
    ctm.AttribPrecision(map, aOptions.mColorPrecision);
  }

  // Set file comment
  if(aMesh->mComment.size() > 0)
    ctm.FileComment(aMesh->mComment.c_str());

  // Set compression method and level
  ctm.CompressionMethod(aOptions.mMethod);
  ctm.CompressionLevel(aOptions.mLevel);

  // Set vertex precision
  if(aOptions.mVertexPrecision > 0.0f)
    ctm.VertexPrecision(aOptions.mVertexPrecision);
  else
    ctm.VertexPrecisionRel(aOptions.mVertexPrecisionRel);

  // Set normal precision
  ctm.NormalPrecision(aOptions.mNormalPrecision);

  // Export file
  ctm.SaveFile(aFileName);
}
