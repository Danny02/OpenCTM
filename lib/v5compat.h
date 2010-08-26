//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        v5compat.h
// Description: Support for version 5 format files (internal).
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

#ifndef __OPENCTM_V5COMPAT_H_
#define __OPENCTM_V5COMPAT_H_

//-----------------------------------------------------------------------------
// _CTMchunklist - Linked data array (represented as a linked list of data
// chunks).
//-----------------------------------------------------------------------------
typedef struct _CTMchunklist_struct _CTMchunklist;
struct _CTMchunklist_struct {
  CTMuint mSize;         // Number of bytes in this data chunk.
  CTMubyte * mData;      // Data array (mSize bytes long).
  _CTMchunklist * mNext; // Pointer to the next chunk in the list (linked list)
};

//-----------------------------------------------------------------------------
// _CTMv5compat - Context for handling v5 specific data and state.
//-----------------------------------------------------------------------------
typedef struct {
  // Chunk list data stream
  _CTMchunklist * mFirstChunk;    // First chunk in the list (0 = empty list)
  _CTMchunklist * mCurrentChunk;  // Current chunk in stream (0 = end of file)
  CTMuint mChunkPos;              // Current offset (relative to mCurrentChunk)

  // Internal state
  CTMuint mMethod;                // Compression method
  CTMuint mVertexCount;           // Vertex count
  CTMuint mTriangleCount;         // Triangle count
  CTMuint mHasNormals;            // Non-zero if normals are present
  CTMuint mUVMapCount;            // UV map count
  CTMuint mAttribMapCount;        // Attribute map count
  _CTMchunklist * mLastHeadChunk; // Last chunk in the file header (used for
                                  // appending the UV map & attrib map info)
} _CTMv5compat;

#endif // __OPENCTM_V5COMPAT_H_
