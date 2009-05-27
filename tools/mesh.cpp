#include <stdexcept>
#include <openctm.h>
#include <cmath>
#include "mesh.h"
#include "convoptions.h"


using namespace std;


/// Compute the cross product of two vectors
Vector3 Cross(Vector3 &v1, Vector3 &v2)
{
  return Vector3(
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x
  );
}

/// Normalize a vector
Vector3 Normalize(Vector3 v)
{
  float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  if(len > 1e-20f)
    len = 1.0f / len;
  else
    len = 1.0f;
  return Vector3(v.x * len, v.y * len, v.z * len);
}


/// Clear the mesh
void Mesh::Clear()
{
  mComment = string("");
  mIndices.clear();
  mVertices.clear();
  mNormals.clear();
  mColors.clear();
  mTexCoords.clear();
}

/// Load the mesh from a file
void Mesh::LoadFromFile(const char * aFileName)
{
  Clear();

  // Load the file using the OpenCTM API
  CTMcontext ctm = ctmNewContext(CTM_IMPORT);
  try
  {
    // Load the file
    ctmLoad(ctm, aFileName);
    if(ctmGetError(ctm) != CTM_NONE)
      throw runtime_error("Unable to load the file.");

    // Extract file comment
    const char * comment = ctmGetString(ctm, CTM_FILE_COMMENT);
    if(comment)
      mComment = string(comment);

    // Extract indices
    CTMuint numTriangles = ctmGetInteger(ctm, CTM_TRIANGLE_COUNT);
    mIndices.resize(numTriangles * 3);
    const CTMuint * indices = ctmGetIntegerArray(ctm, CTM_INDICES);
    for(CTMuint i = 0; i < numTriangles * 3; ++ i)
      mIndices[i] = indices[i];

    // Extract vertices
    CTMuint numVertices = ctmGetInteger(ctm, CTM_VERTEX_COUNT);
    mVertices.resize(numVertices);
    const CTMfloat * vertices = ctmGetFloatArray(ctm, CTM_VERTICES);
    for(CTMuint i = 0; i < numVertices; ++ i)
    {
      mVertices[i].x = vertices[i * 3];
      mVertices[i].y = vertices[i * 3 + 1];
      mVertices[i].z = vertices[i * 3 + 2];
    }

    // Extract normals
    if(ctmGetInteger(ctm, CTM_HAS_NORMALS) == CTM_TRUE)
    {
      mNormals.resize(numVertices);
      const CTMfloat * normals = ctmGetFloatArray(ctm, CTM_NORMALS);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mNormals[i].x = normals[i * 3];
        mNormals[i].y = normals[i * 3 + 1];
        mNormals[i].z = normals[i * 3 + 2];
      }
    }

    // Extract texture coordinates
    if(ctmGetInteger(ctm, CTM_TEX_MAP_COUNT) > 0)
    {
      mTexCoords.resize(numVertices);
      const CTMfloat * texCoords = ctmGetFloatArray(ctm, CTM_TEX_MAP_1);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mTexCoords[i].u = texCoords[i * 2];
        mTexCoords[i].v = texCoords[i * 2 + 1];
      }
    }

    // Extract colors
    CTMenum colorAttrib = ctmGetNamedAttribMap(ctm, "Colors");
    if(colorAttrib != CTM_NONE)
    {
      mColors.resize(numVertices);
      const CTMfloat * colors = ctmGetFloatArray(ctm, colorAttrib);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mColors[i].x = colors[i * 4];
        mColors[i].y = colors[i * 4 + 1];
        mColors[i].z = colors[i * 4 + 2];
        mColors[i].w = colors[i * 4 + 3];
      }
    }

    // We are done with the OpenCTM context
    ctmFreeContext(ctm);
  }
  catch(exception &e)
  {
    ctmFreeContext(ctm);
    throw;
  }
}

/// Save the mesh to a file
void Mesh::SaveToFile(const char * aFileName)
{
  Options opt;
  SaveToFile(aFileName, opt);
}

/// Save the mesh to a file, with options
void Mesh::SaveToFile(const char * aFileName, Options &aOptions)
{
  // Save the file using the OpenCTM API
  CTMcontext ctm = ctmNewContext(CTM_EXPORT);
  try
  {
    // Define mesh
    CTMfloat * normals = 0;
    if(mNormals.size() > 0)
      normals = &mNormals[0].x;
    ctmDefineMesh(ctm, (CTMfloat *) &mVertices[0].x, mVertices.size(),
                  (const CTMuint*) &mIndices[0], mIndices.size() / 3,
                  normals);
    if(ctmGetError(ctm) != CTM_NONE)
      throw runtime_error("Unable to save the file.");

    // Define texture coordinates
    if(mTexCoords.size() > 0)
    {
      CTMenum map = ctmAddTexMap(ctm, &mTexCoords[0].u, "Diffuse color", NULL);
      ctmTexCoordPrecision(ctm, map, aOptions.mTexMapPrecision);
    }

    // Define vertex colors
    if(mColors.size() > 0)
    {
      CTMenum map = ctmAddAttribMap(ctm, &mColors[0].x, "Colors");
      ctmAttribPrecision(ctm, map, aOptions.mColorPrecision);
    }

    // Set file comment
    if(mComment.size() > 0)
      ctmFileComment(ctm, mComment.c_str());

    // Set compression method
    ctmCompressionMethod(ctm, aOptions.mMethod);
    if(aOptions.mVertexPrecision > 0.0f)
      ctmVertexPrecision(ctm, aOptions.mVertexPrecision);
    else
      ctmVertexPrecisionRel(ctm, aOptions.mVertexPrecisionRel);

    // Export file
    ctmSave(ctm, aFileName);

    // Free OpenCTM context
    ctmFreeContext(ctm);
  }
  catch(exception &e)
  {
    ctmFreeContext(ctm);
    throw;
  }
}

/// Calculate smooth per-vertex normals
void Mesh::CalculateNormals()
{
  // Clear the smooth normals
  mNormals.resize(mVertices.size());
  for(unsigned int i = 0; i < mNormals.size(); ++ i)
    mNormals[i] = Vector3(0.0f, 0.0f, 0.0f);

  // Calculate sum of the flat normals of the neighbouring triangles
  unsigned int triCount = mIndices.size() / 3;
  for(unsigned int i = 0; i < triCount; ++ i)
  {
    // Calculate the flat normal for this triangle
    Vector3 v1 = mVertices[mIndices[i * 3 + 1]] - mVertices[mIndices[i * 3]];
    Vector3 v2 = mVertices[mIndices[i * 3 + 2]] - mVertices[mIndices[i * 3]];
    Vector3 flatNormal = Normalize(Cross(v1, v2));

    // ...and add it to all three triangle vertices' smooth normals
    for(unsigned int j = 0; j < 3; ++ j)
      mNormals[mIndices[i * 3 + j]] += flatNormal;
  }

  // Normalize all the smooth normals
  for(unsigned int i = 0; i < mNormals.size(); ++ i)
    mNormals[i] = Normalize(mNormals[i]);
}

/// Calculate the bounding box for the mesh
void Mesh::BoundingBox(Vector3 &aMin, Vector3 &aMax)
{
  if(mVertices.size() < 1)
    aMin = aMax = Vector3(0.0f, 0.0f, 0.0f);
  else
    aMin = aMax = mVertices[0];
  for(unsigned int i = 1; i < mVertices.size(); ++ i)
  {
    if(mVertices[i].x < aMin.x)
      aMin.x = mVertices[i].x;
    else if(mVertices[i].x > aMax.x)
      aMax.x = mVertices[i].x;
    if(mVertices[i].y < aMin.y)
      aMin.y = mVertices[i].y;
    else if(mVertices[i].y > aMax.y)
      aMax.y = mVertices[i].y;
    if(mVertices[i].z < aMin.z)
      aMin.z = mVertices[i].z;
    else if(mVertices[i].z > aMax.z)
      aMax.z = mVertices[i].z;
  }
}
