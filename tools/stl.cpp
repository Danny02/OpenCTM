#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include "stl.h"

#ifdef _MSC_VER
typedef unsigned int uint32;
#else
#include <stdint.h>
typedef uint32_t uint32;
#endif

using namespace std;


/// Read a 32-bit integer, endian independent.
static uint32 ReadInt32(istream &aStream)
{
  unsigned char buf[4];
  aStream.read((char *) buf, 4);
  return ((uint32) buf[0]) | (((uint32) buf[1]) << 8) |
         (((uint32) buf[2]) << 16) | (((uint32) buf[3]) << 24);
}

/// Write a 32-bit integer, endian independent.
static void WriteInt32(ostream &aStream, uint32 aValue)
{
  unsigned char buf[4];
  buf[0] = aValue & 255;
  buf[1] = (aValue >> 8) & 255;
  buf[2] = (aValue >> 16) & 255;
  buf[3] = (aValue >> 24) & 255;
  aStream.write((char *) buf, 4);
}

/// Read a Vector3, endian independent.
static Vector3 ReadVector3(istream &aStream)
{
  union {
    uint32 i;
    float  f;
  } val;
  Vector3 result;
  val.i = ReadInt32(aStream);
  result.x = val.f;
  val.i = ReadInt32(aStream);
  result.y = val.f;
  val.i = ReadInt32(aStream);
  result.z = val.f;
  return result;
}

/// Write a Vector3, endian independent.
static void WriteVector3(ostream &aStream, Vector3 aValue)
{
  union {
    uint32 i;
    float  f;
  } val;
  val.f = aValue.x;
  WriteInt32(aStream, val.i);
  val.f = aValue.y;
  WriteInt32(aStream, val.i);
  val.f = aValue.z;
  WriteInt32(aStream, val.i);
}

/// Vertex class used when reading and joining the triangle vertices.
class SortVertex {
  public:
    float x, y, z;
    uint32 mOldIndex;

    bool operator<(const SortVertex &v) const
    {
      return (x < v.x) || ((x == v.x) && ((y < v.y) || ((y == v.y) && (z < v.z))));
    }
};

/// Import an STL file from a stream.
void STL_Import(istream &aStream, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();

  // Get the file size
  aStream.seekg(0, ios_base::end);
  uint32 fileSize = (uint32) aStream.tellg();
  aStream.seekg(0, ios_base::beg);
  if(fileSize < 84)
    throw runtime_error("Invalid format - not a valid STL file.");

  // Read header (80 character comment + triangle count)
  char comment[81];
  aStream.read(comment, 80);
  comment[80] = 0;
  aMesh.mComment = string(comment);
  uint32 triangleCount = ReadInt32(aStream);
  if(fileSize != (84 + triangleCount * 50))
    throw runtime_error("Invalid format - not a valid STL file.");

  if(triangleCount > 0)
  {
    // Read all the triangle data
    vector<SortVertex> vertices;
    vertices.resize(triangleCount * 3);
    for(uint32 i = 0; i < triangleCount; ++ i)
    {
      // Skip the flat normal
      aStream.seekg(12, ios_base::cur);

      // Read the three triangle vertices
      for(uint32 j = 0; j < 3; ++ j)
      {
        Vector3 v = ReadVector3(aStream);
        uint32 index = i * 3 + j;
        vertices[index].x = v.x;
        vertices[index].y = v.y;
        vertices[index].z = v.z;
        vertices[index].mOldIndex = index;
      }

      // Ignore the two fill bytes
      aStream.seekg(2, ios_base::cur);
    }

    // Make sure that no redundant copies of vertices exist (STL files are full
    // of vertex duplicates, so remove the redundancy), and store the data in
    // the mesh object
    sort(vertices.begin(), vertices.end());
    aMesh.mVertices.resize(vertices.size());
    aMesh.mIndices.resize(vertices.size());
    uint32 vertIdx = 0;
    SortVertex * firstEqual = &vertices[0];
    aMesh.mVertices[vertIdx] = Vector3(vertices[0].x, vertices[0].y, vertices[0].z);
    aMesh.mIndices[vertices[vertIdx].mOldIndex] = vertIdx;
    for(uint32 i = 1; i < vertices.size(); ++ i)
    {
      if((vertices[i].x != firstEqual->x) ||
         (vertices[i].y != firstEqual->y) ||
         (vertices[i].z != firstEqual->z))
      {
        firstEqual = &vertices[i];
        ++ vertIdx;
        aMesh.mVertices[vertIdx] = Vector3(firstEqual->x, firstEqual->y, firstEqual->z);
      }
      aMesh.mIndices[vertices[i].mOldIndex] = vertIdx;
    }
    aMesh.mVertices.resize(vertIdx + 1);
  }
}

/// Export an STL file to a stream.
void STL_Export(ostream &aStream, Mesh &aMesh)
{
  // Write header (80-character comment + triangle count)
  char comment[80];
  for(uint32 i = 0; i < 80; ++ i)
  {
    if(i < aMesh.mComment.size())
      comment[i] = aMesh.mComment[i];
    else
      comment[i] = 0;
  }
  aStream.write(comment, 80);
  uint32 triangleCount = aMesh.mIndices.size() / 3;
  WriteInt32(aStream, triangleCount);

  // Write the triangle data
  for(uint32 i = 0; i < triangleCount; ++ i)
  {
    // Get the triangle vertices
    Vector3 v1 = aMesh.mVertices[aMesh.mIndices[i * 3]];
    Vector3 v2 = aMesh.mVertices[aMesh.mIndices[i * 3 + 1]];
    Vector3 v3 = aMesh.mVertices[aMesh.mIndices[i * 3 + 2]];

    // Calculate the triangle normal
    Vector3 n1 = v2 - v1;
    Vector3 n2 = v3 - v1;
    Vector3 n = Normalize(Cross(n1, n2));

    // Write the triangle normal
    WriteVector3(aStream, n);

    // Coordinates
    WriteVector3(aStream, v1);
    WriteVector3(aStream, v2);
    WriteVector3(aStream, v3);

    // Set the two fill bytes to zero
    aStream.put(0);
    aStream.put(0);
  }
}
