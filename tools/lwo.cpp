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

/// Import a mesh from an LWO file.
void Import_LWO(const char * aFileName, Mesh * aMesh)
{
  throw runtime_error("LWO import has not yet been implemented.");
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
}
