//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        lwo.cpp
// Description: Implementation of the LWO file format importer/exporter.
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
#include <cstring>
#include "lwo.h"

#ifdef _MSC_VER
  typedef unsigned int uint32;
#else
  #include <stdint.h>
  typedef uint32_t uint32;
#endif

using namespace std;


//------------------------------------------------------------------------------
// Helper functions for reading
//------------------------------------------------------------------------------

/// Read a 16-bit integer, endian independent.
static uint32 ReadU2(istream &aStream)
{
  unsigned char buf[2];
  aStream.read((char *) buf, 2);
  return (((uint32) buf[0]) << 8) |
         ((uint32) buf[1]);
}

/// Read a 32-bit integer, endian independent.
static uint32 ReadU4(istream &aStream)
{
  unsigned char buf[4];
  aStream.read((char *) buf, 4);
  return (((uint32) buf[0]) << 24) |
         (((uint32) buf[1]) << 16) |
         (((uint32) buf[2]) << 8) |
         ((uint32) buf[3]);
}

/// Read a 3 x 32-bit floating point vector, endian independent.
static Vector3 ReadVEC12(istream &aStream)
{
  unsigned char buf[12];
  Vector3 result;
  aStream.read((char *) buf, 12);
  union {
    uint32 i;
    float  f;
  } val;
  val.i = (((uint32) buf[0]) << 24) |
          (((uint32) buf[1]) << 16) |
          (((uint32) buf[2]) << 8) |
          ((uint32) buf[3]);
  result.x = val.f;
  val.i = (((uint32) buf[4]) << 24) |
          (((uint32) buf[5]) << 16) |
          (((uint32) buf[6]) << 8) |
          ((uint32) buf[7]);
  result.y = val.f;
  val.i = (((uint32) buf[8]) << 24) |
          (((uint32) buf[9]) << 16) |
          (((uint32) buf[10]) << 8) |
          ((uint32) buf[11]);
  result.z = val.f;
  return result;
}

/// Read a non-terminated string from the stream (e.g. a chunk ID string).
static string ReadString(istream &aStream, int aCount)
{
  string result;
  result.resize(aCount);
  aStream.read((char *) &result[0], aCount);
  return result;
}

/// Read a zero terminated string from a stream.
static string ReadStringZ(istream &aStream)
{
  return string(""); // FIXME
}

/// Read a polygon index (variable size) from a stream.
static uint32 ReadPOLSIndex(istream &aStream, int * aBytesLeft)
{
  uint32 result = ReadU2(aStream);
  if(result >= 0x0000ff00)
  {
    result = ((result & 255) << 16) | ReadU2(aStream);
    *aBytesLeft -= 4;
  }
  else
    *aBytesLeft -= 2;
  return result;
}


//------------------------------------------------------------------------------
// Helper functions for writing
//------------------------------------------------------------------------------

/// Write a pad byte if necessary (align chunks on even byte offsets)
static void WritePadByte(ostream &aStream)
{
  if(aStream.tellp() & 1)
  {
    unsigned char buf[1];
    buf[0] = 0;
    aStream.write((char *) buf, 1);
  }
}

/// Write a 16-bit integer, endian independent.
static void WriteU2(ostream &aStream, uint32 aValue)
{
  unsigned char buf[2];
  buf[0] = (aValue >> 8) & 255;
  buf[1] = aValue & 255;
  aStream.write((char *) buf, 2);
}

/// Write a 32-bit integer, endian independent.
static void WriteU4(ostream &aStream, uint32 aValue)
{
  unsigned char buf[4];
  buf[0] = (aValue >> 24) & 255;
  buf[1] = (aValue >> 16) & 255;
  buf[2] = (aValue >> 8) & 255;
  buf[3] = aValue & 255;
  aStream.write((char *) buf, 4);
}

/// Write a 3 x 32-bit floating point vector, endian independent.
static void WriteVEC12(ostream &aStream, Vector3 aValue)
{
  unsigned char buf[12];
  union {
    uint32 i;
    float  f;
  } val;
  val.f = aValue.x;
  buf[0] = (val.i >> 24) & 255;
  buf[1] = (val.i >> 16) & 255;
  buf[2] = (val.i >> 8) & 255;
  buf[3] = val.i & 255;
  val.f = aValue.y;
  buf[4] = (val.i >> 24) & 255;
  buf[5] = (val.i >> 16) & 255;
  buf[6] = (val.i >> 8) & 255;
  buf[7] = val.i & 255;
  val.f = aValue.z;
  buf[8] = (val.i >> 24) & 255;
  buf[9] = (val.i >> 16) & 255;
  buf[10] = (val.i >> 8) & 255;
  buf[11] = val.i & 255;
  aStream.write((char *) buf, 12);
}

/// Write a string to a stream (no zero termination).
static void WriteString(ostream &aStream, const char * aString)
{
  int len = strlen(aString);
  aStream.write(aString, len);
}

/// Write a zero terminated string to a stream.
static void WriteStringZ(ostream &aStream, const char * aString)
{
  int len = strlen(aString);
  aStream.write(aString, len + 1);
}

/// Calculate the size of a POLS chunk - take variable size indices into
/// account...
static uint32 CalcPOLSSize(Mesh * aMesh)
{
  uint32 triCount = (uint32) (aMesh->mIndices.size() / 3);
  uint32 size = 4 + triCount * 2;
  for(unsigned int i = 0; i < aMesh->mIndices.size(); ++ i)
  {
    uint32 idx = aMesh->mIndices[i];
    if(idx < 0x0000ff00)
      size += 2;
    else
      size += 4;
  }
  return size;
}


//------------------------------------------------------------------------------
// Public functions
//------------------------------------------------------------------------------

/// Import a mesh from an LWO file.
void Import_LWO(const char * aFileName, Mesh * aMesh)
{
  // Open the input file
  ifstream f(aFileName, ios_base::in | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open input file.");

  // File header
  if(ReadString(f, 4) != string("FORM"))
    throw runtime_error("Not a valid LWO file (missing FORM chunk).");
  uint32 fileSize = ReadU4(f);
  if(ReadString(f, 4) != string("LWO2"))
    throw runtime_error("Not a valid LWO file (not LWO2 format).");

  // Start with an empty mesh
  aMesh->Clear();
  uint32 pointCount = 0;
  uint32 triangleCount = 0;
  uint32 indexBias = 0;
  bool havePoints = false;

  // Iterate all chunks
  while(!f.eof() && (f.tellg() < fileSize))
  {
    // Get chunk ID & size
    string chunkID = ReadString(f, 4);
    uint32 chunkSize = ReadU4(f);

    // Was this a supported chunk?
    if(chunkID == string("PNTS"))
    {
      // Check point count
      uint32 newPoints = chunkSize / 12;
      if((newPoints * 12) != chunkSize)
        throw runtime_error("Not a valid LWO file (invalid PNTS chunk).");

      // Read points
      aMesh->mVertices.resize(pointCount + newPoints);
      for(uint32 i = pointCount; i < (uint32) aMesh->mVertices.size(); ++ i)
        aMesh->mVertices[i] = ReadVEC12(f);
      indexBias = pointCount;
      pointCount += newPoints;
      havePoints = true;
    }
    else if(chunkID == string("POLS"))
    {
      // POLS before PNTS?
      if(!havePoints)
        throw runtime_error("Not a valid LWO file (POLS chunk before PNTS chunk).");

      // Check that we have a FACE descriptor
      if(ReadString(f, 4) == string("FACE"))
      {
        // Perpare for worst case triangle count (a single poly with only
        // 16-bit indices)
        uint32 maxTriCount = (chunkSize - 10) / 2;
        vector<uint32> indices;
        indices.resize(maxTriCount * 3);

        // Read polygons
        uint32 newTris = 0;
        int bytesLeft = (int) chunkSize - 4;
        while(bytesLeft > 0)
        {
          int polyNodes = (int) ReadU2(f);
          bytesLeft -= 2;
          if(polyNodes >= 3)
          {
            polyNodes -= 3;
            uint32 idx[3];
            idx[0] = ReadPOLSIndex(f, &bytesLeft);
            idx[1] = ReadPOLSIndex(f, &bytesLeft);
            idx[2] = ReadPOLSIndex(f, &bytesLeft);
            while((polyNodes >= 0) && (bytesLeft >= 0))
            {
              indices[newTris * 3] = idx[0];
              indices[newTris * 3 + 1] = idx[1];
              indices[newTris * 3 + 2] = idx[2];
              ++ newTris;
              if(polyNodes > 0)
              {
                idx[1] = idx[2];
                idx[2] = ReadPOLSIndex(f, &bytesLeft);
              }
              -- polyNodes;
            }
          }
          else
          {
            // Skip polygons with less than 3 nodes
            for(int i = 0; i < polyNodes; ++ i)
              ReadPOLSIndex(f, &bytesLeft);
          }
        }

        // Copy all the read indices to the mesh
        aMesh->mIndices.resize((triangleCount + newTris) * 3);
        for(uint32 i = 0; i < newTris; ++ i)
        {
          aMesh->mIndices[(i + triangleCount) * 3] = indices[i * 3] + indexBias;
          aMesh->mIndices[(i + triangleCount) * 3 + 1] = indices[i * 3 + 1] + indexBias;
          aMesh->mIndices[(i + triangleCount) * 3 + 2] = indices[i * 3 + 2] + indexBias;
        }
        triangleCount += newTris;
      }
      else
      {
        // We only support FACE type polygons - skip this chunk
        f.seekg(((chunkSize + 1) & 0x7ffffffe) - 4, ios_base::cur);
      }
    }
    else
    {
      // Just skip this chunk (always round to next nearest even offset)
      f.seekg((chunkSize + 1) & 0x7ffffffe, ios_base::cur);
    }
  }

  // Close the input file
  f.close();
}

/// Export a mesh to an LWO file.
void Export_LWO(const char * aFileName, Mesh * aMesh)
{
  // Check if we can support this mesh (too many vertices?)
  if(aMesh->mVertices.size() > 0x00ffffff)
    throw runtime_error("Too large mesh (not supported by the LWO file format).");

  // Calculate output file size
  uint32 pntsSize = (uint32) (aMesh->mVertices.size() * 12);
  uint32 tagsSize = 8;
  uint32 layrSize = 18;
  uint32 polsSize = CalcPOLSSize(aMesh);
  uint32 fileSize = 4 +
                    8 + tagsSize +
                    8 + layrSize +
                    8 + pntsSize +
                    8 + polsSize;

  // Open the output file
  ofstream f(aFileName, ios_base::out | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open output file.");

  // File header
  WriteString(f, "FORM");
  WriteU4(f, fileSize);      // File size (excluding FORM chunk header)
  WriteString(f, "LWO2");

  // TAGS chunk
  WriteString(f, "TAGS");
  WriteU4(f, tagsSize);
  WriteStringZ(f, "Default");
  WritePadByte(f);

  // LAYR chunk
  WriteString(f, "LAYR");
  WriteU4(f, layrSize);
  WriteU2(f, 0);                            // number
  WriteU2(f, 0);                            // flags
  WriteVEC12(f, Vector3(0.0f, 0.0f, 0.0f)); // pivot
  WriteStringZ(f, "");                      // name
  WritePadByte(f);

  // PNTS chunk
  WriteString(f, "PNTS");
  WriteU4(f, pntsSize);
  for(uint32 i = 0; i < (uint32) aMesh->mVertices.size(); ++ i)
    WriteVEC12(f, aMesh->mVertices[i]);
  WritePadByte(f);

  // POLS chunk
  WriteString(f, "POLS");
  WriteU4(f, polsSize);
  WriteString(f, "FACE");
  uint32 triCount = (uint32) (aMesh->mIndices.size() / 3);
  for(uint32 i = 0; i < triCount; ++ i)
  {
    // Polygon node count (always 3)
    WriteU2(f, 3);

    // Write polygon node indices
    for(int j = 0; j < 3; ++ j)
    {
      uint32 idx = aMesh->mIndices[i * 3 + j];
      if(idx < 0x0000ff00)
        WriteU2(f, idx);
      else
        WriteU4(f, idx + 0xff000000);
    }
  }
  WritePadByte(f);

  // Close the output file
  f.close();
}
