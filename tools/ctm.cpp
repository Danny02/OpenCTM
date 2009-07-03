#include <stdexcept>
#include <openctm.h>
#include "ctm.h"

using namespace std;


/// Import an OpenCTM file from a file.
void Import_CTM(const char * aFileName, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();

  // Load the file using the OpenCTM API
  CTMimporter ctm;

  // Load the file
  ctm.Load(aFileName);

  // Extract file comment
  const char * comment = ctm.GetString(CTM_FILE_COMMENT);
  if(comment)
    aMesh.mComment = string(comment);

  // Extract indices
  CTMuint numTriangles = ctm.GetInteger(CTM_TRIANGLE_COUNT);
  aMesh.mIndices.resize(numTriangles * 3);
  const CTMuint * indices = ctm.GetIntegerArray(CTM_INDICES);
  for(CTMuint i = 0; i < numTriangles * 3; ++ i)
    aMesh.mIndices[i] = indices[i];

  // Extract vertices
  CTMuint numVertices = ctm.GetInteger(CTM_VERTEX_COUNT);
  aMesh.mVertices.resize(numVertices);
  const CTMfloat * vertices = ctm.GetFloatArray(CTM_VERTICES);
  for(CTMuint i = 0; i < numVertices; ++ i)
  {
    aMesh.mVertices[i].x = vertices[i * 3];
    aMesh.mVertices[i].y = vertices[i * 3 + 1];
    aMesh.mVertices[i].z = vertices[i * 3 + 2];
  }

  // Extract normals
  if(ctm.GetInteger(CTM_HAS_NORMALS) == CTM_TRUE)
  {
    aMesh.mNormals.resize(numVertices);
    const CTMfloat * normals = ctm.GetFloatArray(CTM_NORMALS);
    for(CTMuint i = 0; i < numVertices; ++ i)
    {
      aMesh.mNormals[i].x = normals[i * 3];
      aMesh.mNormals[i].y = normals[i * 3 + 1];
      aMesh.mNormals[i].z = normals[i * 3 + 2];
    }
  }

  // Extract texture coordinates
  if(ctm.GetInteger(CTM_TEX_MAP_COUNT) > 0)
  {
    aMesh.mTexCoords.resize(numVertices);
    const CTMfloat * texCoords = ctm.GetFloatArray(CTM_TEX_MAP_1);
    for(CTMuint i = 0; i < numVertices; ++ i)
    {
      aMesh.mTexCoords[i].u = texCoords[i * 2];
      aMesh.mTexCoords[i].v = texCoords[i * 2 + 1];
    }
    const char * str = ctm.GetTexMapString(CTM_TEX_MAP_1, CTM_FILE_NAME);
    if(str)
      aMesh.mTexFileName = string(str);
    else
      aMesh.mTexFileName = string("");
  }

  // Extract colors
  CTMenum colorAttrib = ctm.GetNamedAttribMap("Colors");
  if(colorAttrib != CTM_NONE)
  {
    aMesh.mColors.resize(numVertices);
    const CTMfloat * colors = ctm.GetFloatArray(colorAttrib);
    for(CTMuint i = 0; i < numVertices; ++ i)
    {
      aMesh.mColors[i].x = colors[i * 4];
      aMesh.mColors[i].y = colors[i * 4 + 1];
      aMesh.mColors[i].z = colors[i * 4 + 2];
      aMesh.mColors[i].w = colors[i * 4 + 3];
    }
  }
}

/// Export an OpenCTM file to a file.
void Export_CTM(const char * aFileName, Mesh &aMesh, Options &aOptions)
{
  // Save the file using the OpenCTM API
  CTMexporter ctm;

  // Define mesh
  CTMfloat * normals = 0;
  if(aMesh.mNormals.size() > 0)
    normals = &aMesh.mNormals[0].x;
  ctm.DefineMesh((CTMfloat *) &aMesh.mVertices[0].x, aMesh.mVertices.size(),
                 (const CTMuint*) &aMesh.mIndices[0], aMesh.mIndices.size() / 3,
                 normals);

  // Define texture coordinates
  if(aMesh.mTexCoords.size() > 0)
  {
    const char * fileName = NULL;
    if(aMesh.mTexFileName.size() > 0)
      fileName = aMesh.mTexFileName.c_str();
    CTMenum map = ctm.AddTexMap(&aMesh.mTexCoords[0].u, "Diffuse color", fileName);
    ctm.TexCoordPrecision(map, aOptions.mTexMapPrecision);
  }

  // Define vertex colors
  if(aMesh.mColors.size() > 0)
  {
    CTMenum map = ctm.AddAttribMap(&aMesh.mColors[0].x, "Colors");
    ctm.AttribPrecision(map, aOptions.mColorPrecision);
  }

  // Set file comment
  if(aMesh.mComment.size() > 0)
    ctm.FileComment(aMesh.mComment.c_str());

  // Set compression method
  ctm.CompressionMethod(aOptions.mMethod);

  // Set vertex precision
  if(aOptions.mVertexPrecision > 0.0f)
    ctm.VertexPrecision(aOptions.mVertexPrecision);
  else
    ctm.VertexPrecisionRel(aOptions.mVertexPrecisionRel);

  // Set normal precision
  ctm.NormalPrecision(aOptions.mNormalPrecision);

  // Export file
  ctm.Save(aFileName);
}
