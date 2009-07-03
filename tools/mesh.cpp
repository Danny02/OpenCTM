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
  mTexFileName = string("");
  mIndices.clear();
  mVertices.clear();
  mNormals.clear();
  mColors.clear();
  mTexCoords.clear();
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
