#include <stdexcept>
#include <vector>
#include <list>
#include "3ds.h"

#ifdef _MSC_VER
typedef unsigned short uint16;
typedef unsigned int uint32;
#else
#include <stdint.h>
typedef uint16_t uint16;
typedef uint32_t uint32;
#endif

using namespace std;


// Known 3DS chunks
#define CHUNK_MAIN          0x4d4d
#define CHUNK_3DEDIT        0x3d3d
#define CHUNK_OBJECT        0x4000
#define CHUNK_TRIMESH       0x4100
#define CHUNK_VERTEXLIST    0x4110
#define CHUNK_FACES         0x4120
#define CHUNK_MAPPINGCOORDS 0x4140
#define CHUNK_TRANSFORM     0x4160
#define CHUNK_MASTERSCALE   0x0100

// 3DS object class
class Obj3DS {
  public:
    vector<uint16> mIndices;
    vector<Vector3> mVertices;
    vector<Vector2> mUVCoords;
};


/// Read a 16-bit integer, endian independent.
static uint16 ReadInt16(istream &aStream)
{
  unsigned char buf[2];
  aStream.read((char *) buf, 2);
  return ((uint16) buf[0]) | (((uint16) buf[1]) << 8);
}

/// Write a 16-bit integer, endian independent.
static void WriteInt16(ostream &aStream, uint16 aValue)
{
  unsigned char buf[2];
  buf[0] = aValue & 255;
  buf[1] = (aValue >> 8) & 255;
  aStream.write((char *) buf, 2);
}

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

/// Read a Vector2, endian independent.
static Vector2 ReadVector2(istream &aStream)
{
  union {
    uint32 i;
    float  f;
  } val;
  Vector2 result;
  val.i = ReadInt32(aStream);
  result.u = val.f;
  val.i = ReadInt32(aStream);
  result.v = val.f;
  return result;
}

/// Write a Vector2, endian independent.
static void WriteVector2(ostream &aStream, Vector2 aValue)
{
  union {
    uint32 i;
    float  f;
  } val;
  val.f = aValue.u;
  WriteInt32(aStream, val.i);
  val.f = aValue.v;
  WriteInt32(aStream, val.i);
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

/// Import a 3DS file from a stream.
void Import_3DS(istream &aStream, Mesh &aMesh)
{
  uint16 chunk, count;
  uint32 chunkLen;

  // Get file size
  aStream.seekg(0, ios_base::end);
  uint32 fileSize = aStream.tellg();
  aStream.seekg(0, ios_base::beg);

  // Check file size (rough initial check)
  if(fileSize < 6)
    throw runtime_error("Invalid 3DS file format.");

  // Read & check file header identifier
  chunk = ReadInt16(aStream);
  chunkLen = ReadInt32(aStream);
  if((chunk != CHUNK_MAIN) || (chunkLen != fileSize))
    throw runtime_error("Invalid 3DS file format.");

  // Parse chunks, and store the data in a temporary list, objList...
  Obj3DS * obj = 0;
  list<Obj3DS> objList;
  bool hasUVCoords = false;
  while(aStream.tellg() < fileSize)
  {
    // Read next chunk
    chunk = ReadInt16(aStream);
    chunkLen = ReadInt32(aStream);

    // What chunk did we get?
    switch(chunk)
    {
      // 3D Edit -> Step into
      case CHUNK_3DEDIT:
        break;

      // Object -> Step into
      case CHUNK_OBJECT:
        // Skip object name (null terminated string)
        while((aStream.tellg() < fileSize) && aStream.get());

        // Create a new object
        objList.push_back(Obj3DS());
        obj = &objList.back();
        break;

      // Triangle mesh -> Step into
      case CHUNK_TRIMESH:
        break;

      // Vertex list (point coordinates)
      case CHUNK_VERTEXLIST:
        count = ReadInt16(aStream);
        if((!obj) || ((obj->mVertices.size() > 0) && (obj->mVertices.size() != count)))
        {
          aStream.seekg(count * 12, ios_base::cur);
          break;
        }
        if(obj->mVertices.size() == 0)
          obj->mVertices.resize(count);
        for(uint16 i = 0; i < count; ++ i)
          obj->mVertices[i] = ReadVector3(aStream);
        break;

      // Texture map coordinates (UV coordinates)
      case CHUNK_MAPPINGCOORDS:
        count = ReadInt16(aStream);
        if((!obj) || ((obj->mUVCoords.size() > 0) && (obj->mUVCoords.size() != count)))
        {
          aStream.seekg(count * 8, ios_base::cur);
          break;
        }
        if(obj->mUVCoords.size() == 0)
          obj->mUVCoords.resize(count);
        for(uint16 i = 0; i < count; ++ i)
          obj->mUVCoords[i] = ReadVector2(aStream);
        if(count > 0)
          hasUVCoords = true;
        break;

      // Face description (triangle indices)
      case CHUNK_FACES:
        count = ReadInt16(aStream);
        if(!obj)
        {
          aStream.seekg(count * 8, ios_base::cur);
          break;
        }
        if(obj->mIndices.size() == 0)
          obj->mIndices.resize(3 * count);
        for(uint32 i = 0; i < count; ++ i)
        {
          obj->mIndices[i * 3] = ReadInt16(aStream);
          obj->mIndices[i * 3 + 1] = ReadInt16(aStream);
          obj->mIndices[i * 3 + 2] = ReadInt16(aStream);
          ReadInt16(aStream); // Skip face flag
        }
        break;
        
      default:      // Unknown/ignored - skip past this one
        aStream.seekg(chunkLen - 6, ios_base::cur);
    }
  }

  // Convert the loaded object list to the mesh structore (merge all geometries)
  aMesh.Clear();
  for(list<Obj3DS>::iterator o = objList.begin(); o != objList.end(); ++ o)
  {
    // Append...
    uint32 idxOffset = aMesh.mIndices.size();
    uint32 vertOffset = aMesh.mVertices.size();
    aMesh.mIndices.resize(idxOffset + (*o).mIndices.size());
    aMesh.mVertices.resize(vertOffset + (*o).mVertices.size());
    if(hasUVCoords)
      aMesh.mTexCoords.resize(vertOffset + (*o).mVertices.size());

    // Transcode the data
    for(uint32 i = 0; i < (*o).mIndices.size(); ++ i)
      aMesh.mIndices[idxOffset + i] = vertOffset + uint32((*o).mIndices[i]);
    for(uint32 i = 0; i < (*o).mVertices.size(); ++ i)
      aMesh.mVertices[vertOffset + i] = (*o).mVertices[i];
    if(hasUVCoords)
    {
      if((*o).mUVCoords.size() == (*o).mVertices.size())
        for(uint32 i = 0; i < (*o).mVertices.size(); ++ i)
          aMesh.mTexCoords[vertOffset + i] = (*o).mUVCoords[i];
      else
        for(uint32 i = 0; i < (*o).mVertices.size(); ++ i)
          aMesh.mTexCoords[vertOffset + i] = Vector2(0.0f, 0.0f);
    }
  }
}

/// Export a 3DS file to a stream.
void Export_3DS(ostream &aStream, Mesh &aMesh)
{
  // FIXME!
  throw runtime_error("No support for 3DS export yet.");
}
