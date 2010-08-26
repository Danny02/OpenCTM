//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        v5compat.c
// Description: Support for loading version 5 format files (Note: no support
//              for saving version 5 format files is provided).
//
//              The basic idea is to load the version 5 file into a memory
//              buffer, while converting it to version 6 format. The rest of
//              the OpenCTM library will read the file from the memory buffer
//              instead of from the file.
//
//              This solution was chosen for a few good reasons:
//               1) In v5 files, the UV map and attribute map names are stored
//                  along with the UV/attrib map data (late in the file stream)
//                  while the v6 format reader requires the data to be
//                  available in the file header (early in the file).
//               2) The API does not support seek:ing in the source data
//                  stream (by design), so the entire file has to be read into
//                  memory to make it seek:able.
//               3) The v6 format reader should not be cluttered with v5
//                  compatibility hacks.
//
//              Also, some extra necessary conversion functions are implemented
//              here that solves differences between the v5 and v6 uncompressed
//              data formats (called from the v6 reader as post-processing
//              where necessary, in order to avoid having to decode/encode the
//              data with LZMA during the v5 load-to-memory process).
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

#include <stdlib.h>
#include <string.h>
#include "openctm2.h"
#include "internal.h"

#ifdef _CTM_SUPPORT_V5_FILES

//=============================================================================
// Memory chunk list management functions
//=============================================================================

//-----------------------------------------------------------------------------
// _ctmNewMemChunk() - Allocate memory for a new chunk.
//-----------------------------------------------------------------------------
static _CTMchunklist * _ctmNewMemChunk(CTMuint aSize)
{
  _CTMchunklist *chunk;

  // Allocate memory for the object
  chunk = (_CTMchunklist *) malloc(sizeof(_CTMchunklist));
  if(!chunk)
    return (_CTMchunklist *) 0;

  // Initialize the object
  chunk->mData = (CTMubyte *) malloc(aSize);
  if(!chunk->mData)
  {
    free((void *) chunk);
    return (_CTMchunklist *) 0;
  }
  chunk->mSize = aSize;
  chunk->mNext = (_CTMchunklist *) 0;

  return chunk;
}

//-----------------------------------------------------------------------------
// _ctmInsertMemChunk() - Insert the given chunk after aPrevious (insert
// it before the next item).
//-----------------------------------------------------------------------------
static void _ctmInsertMemChunk(_CTMchunklist * aPrevious,
  _CTMchunklist * aChunk)
{
  aChunk->mNext = aPrevious->mNext;
  aPrevious->mNext = aChunk;
}

//-----------------------------------------------------------------------------
// _ctmAppendMemChunkLast() - Append the given chunk at the end of the chunk
// list.
//-----------------------------------------------------------------------------
static void _ctmAppendMemChunkLast(_CTMchunklist ** aList,
  _CTMchunklist * aChunk)
{
  _CTMchunklist **chunkptr = aList;
  while(*chunkptr)
    chunkptr = &(*chunkptr)->mNext;
  *chunkptr = aChunk;
}

//-----------------------------------------------------------------------------
// _ctmSetUINT() - Set an integer value in a byte buffer.
//-----------------------------------------------------------------------------
static void _ctmSetUINT(CTMubyte * aBuf, CTMuint aValue)
{
  aBuf[0] = (CTMubyte)aValue;
  aBuf[1] = (CTMubyte)(aValue >> 8);
  aBuf[2] = (CTMubyte)(aValue >> 16);
  aBuf[3] = (CTMubyte)(aValue >> 24);
}

//-----------------------------------------------------------------------------
// _ctmMemChunkRead() - Read function for reading the chunk list as a stream.
//-----------------------------------------------------------------------------
static CTMuint CTMCALL _ctmMemChunkRead(void * aBuf, CTMuint aCount,
  void * aUserData)
{
  CTMuint bytesRead, pos;
  CTMubyte *dst, *src;
  _CTMv5compat * v5compat = (_CTMv5compat *) aUserData;
  _CTMchunklist * chunk;

  // End of stream?
  if(!v5compat->mCurrentChunk)
    return 0;

  // Copy the required number of bytes to aBuf
  chunk = v5compat->mCurrentChunk;
  pos = v5compat->mChunkPos;
  dst = (CTMubyte *) aBuf;
  src = chunk->mData;
  for(bytesRead = 0; bytesRead < aCount; ++ bytesRead)
  {
    // Copy one byte
    *dst ++ = *src ++;
    ++ pos;

    // End of the current chunk?
    if(pos >= chunk->mSize)
    {
      chunk = chunk->mNext;
      pos = 0;
      if(chunk)
        src = chunk->mData;
      else
        break;
    }
  }

  // Remember position...
  v5compat->mCurrentChunk = chunk;
  v5compat->mChunkPos = pos;

  return bytesRead;
}


//=============================================================================
// Version 5 format parsing functions
//=============================================================================

//-----------------------------------------------------------------------------
// _ctmLoadV5_Header() - Load file header.
//-----------------------------------------------------------------------------
static int _ctmLoadV5_Header(_CTMcontext * self)
{
  _CTMchunklist * chunk;
  CTMuint len;
  CTMuint method, vertexCount, triangleCount, uvMapCount, attribMapCount;
  CTMuint flags, frameCount;
  char * fileComment;

  // Load file header information
  method = _ctmStreamReadUINT(self);
  vertexCount = _ctmStreamReadUINT(self);
  triangleCount = _ctmStreamReadUINT(self);
  uvMapCount = _ctmStreamReadUINT(self);
  attribMapCount = _ctmStreamReadUINT(self);
  flags = _ctmStreamReadUINT(self);
  frameCount = 1;

  // Check which method we are using
  if(method == FOURCC("RAW\0"))
    self->mV5Compat.mMethod = CTM_METHOD_RAW;
  else if(method == FOURCC("MG1\0"))
    self->mV5Compat.mMethod = CTM_METHOD_MG1;
  else if(method == FOURCC("MG2\0"))
    self->mV5Compat.mMethod = CTM_METHOD_MG2;
  else
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }

  // Sanity check...
  if((vertexCount == 0) || (triangleCount == 0))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }

  // Load file comment (Note: we want to load this after doing initial
  // integrity checking, to avoid trying to load a potentially very long
  // bogus string if this is a badly formatted file).
  len = _ctmStreamReadSTRING(self, &fileComment);

  // Construct v6 format header (excluding UV and attrib map info)
  if(!(chunk = _ctmNewMemChunk(32 + len)))
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  _ctmAppendMemChunkLast(&self->mV5Compat.mFirstChunk, chunk);
  _ctmSetUINT(&chunk->mData[0], method);
  _ctmSetUINT(&chunk->mData[4], vertexCount);
  _ctmSetUINT(&chunk->mData[8], triangleCount);
  _ctmSetUINT(&chunk->mData[12], uvMapCount);
  _ctmSetUINT(&chunk->mData[16], attribMapCount);
  _ctmSetUINT(&chunk->mData[20], (flags & 0x00000001) ? _CTM_HAS_NORMALS_BIT : 0);
  _ctmSetUINT(&chunk->mData[24], frameCount);
  _ctmSetUINT(&chunk->mData[28], len);
  memcpy((void *) &chunk->mData[32], (void *) fileComment, len);
  free((void *) fileComment);

  // Here is where we want to insert UV and attrib map info later on...
  self->mV5Compat.mLastHeadChunk = chunk;

  // Remember vital header information for later on...
  self->mV5Compat.mVertexCount = vertexCount;
  self->mV5Compat.mTriangleCount = triangleCount;
  self->mV5Compat.mHasNormals = (flags & 0x00000001);
  self->mV5Compat.mUVMapCount = uvMapCount;
  self->mV5Compat.mAttribMapCount = attribMapCount;

  return CTM_TRUE;
}

//-----------------------------------------------------------------------------
// _ctmLoadV5_RAW() - Load RAW format mesh.
//-----------------------------------------------------------------------------
static int _ctmLoadV5_RAW(_CTMcontext * self)
{
#ifdef _CTM_SUPPORT_RAW
  _CTMchunklist * chunk;
  CTMuint len, i;

  // Read triangle indices
  if(_ctmStreamReadUINT(self) != FOURCC("INDX"))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }
  len = self->mV5Compat.mTriangleCount * 3 * 4;
  if(!(chunk = _ctmNewMemChunk(4 + len)))
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  _ctmAppendMemChunkLast(&self->mV5Compat.mFirstChunk, chunk);
  _ctmSetUINT(&chunk->mData[0], FOURCC("INDX"));
  _ctmStreamRead(self, &chunk->mData[4], len);

  // Read vertex coordinates
  if(_ctmStreamReadUINT(self) != FOURCC("VERT"))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }
  len = self->mV5Compat.mVertexCount * 3 * 4;
  if(!(chunk = _ctmNewMemChunk(4 + len)))
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  _ctmAppendMemChunkLast(&self->mV5Compat.mFirstChunk, chunk);
  _ctmSetUINT(&chunk->mData[0], FOURCC("VERT"));
  _ctmStreamRead(self, &chunk->mData[4], len);

  // Read normals
  if(self->mV5Compat.mHasNormals)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("NORM"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    len = self->mV5Compat.mVertexCount * 3 * 4;
    if(!(chunk = _ctmNewMemChunk(4 + len)))
    {
      self->mError = CTM_OUT_OF_MEMORY;
      return CTM_FALSE;
    }
    _ctmAppendMemChunkLast(&self->mV5Compat.mFirstChunk, chunk);
    _ctmSetUINT(&chunk->mData[0], FOURCC("NORM"));
    _ctmStreamRead(self, &chunk->mData[4], len);
  }

  // Read UV maps
  for(i = 0; i < self->mV5Compat.mUVMapCount; ++ i)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("TEXC"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
/*
    _ctmStreamReadSTRING(self, &map->mName);
    _ctmStreamReadSTRING(self, &map->mFileName);
    for(i = 0; i < self->mVertexCount * 2; ++ i)
      map->mValues[i] = _ctmStreamReadFLOAT(self);
*/
  }

  // Read attribute maps
  for(i = 0; i < self->mV5Compat.mUVMapCount; ++ i)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("ATTR"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
/*
    _ctmStreamReadSTRING(self, &map->mName);
    _ctmStreamReadSTRING(self, &map->mFileName);
    for(i = 0; i < self->mVertexCount * 2; ++ i)
      map->mValues[i] = _ctmStreamReadFLOAT(self);
*/
  }

  return CTM_TRUE;
#else
  self->mError = CTM_UNSUPPORTED_OPERATION;
  return CTM_FALSE;
#endif
}

//-----------------------------------------------------------------------------
// _ctmLoadV5_MG1() - Load MG1 format mesh.
//-----------------------------------------------------------------------------
static int _ctmLoadV5_MG1(_CTMcontext * self)
{
#ifdef _CTM_SUPPORT_MG1
  // FIXME!
  self->mError = CTM_UNSUPPORTED_OPERATION;
  return CTM_FALSE;
#else
  self->mError = CTM_UNSUPPORTED_OPERATION;
  return CTM_FALSE;
#endif
}

//-----------------------------------------------------------------------------
// _ctmLoadV5_MG2() - Load MG2 format mesh.
//-----------------------------------------------------------------------------
static int _ctmLoadV5_MG2(_CTMcontext * self)
{
#ifdef _CTM_SUPPORT_MG2
  // FIXME!
  self->mError = CTM_UNSUPPORTED_OPERATION;
  return CTM_FALSE;
#else
  self->mError = CTM_UNSUPPORTED_OPERATION;
  return CTM_FALSE;
#endif
}


//=============================================================================
// Public functions (API internal)
//=============================================================================

//-----------------------------------------------------------------------------
// _ctmLoadV5FileToMem() - Load a v5 file into memory. While loading the data,
// convert it to the file format version that is supported by the current
// library.
//-----------------------------------------------------------------------------
int _ctmLoadV5FileToMem(_CTMcontext * self)
{
  int ok;

  // Un-initialize if necessary
  _ctmCleanupV5Data(self);

  // Load file header
  if(!_ctmLoadV5_Header(self))
    return CTM_FALSE;

  // Load the rest of the file (compression method dependent)
  switch(self->mV5Compat.mMethod)
  {
    case CTM_METHOD_RAW:
      ok = _ctmLoadV5_RAW(self);
      break;
    case CTM_METHOD_MG1:
      ok = _ctmLoadV5_MG1(self);
      break;
    case CTM_METHOD_MG2:
      ok = _ctmLoadV5_MG2(self);
      break;
    default:
      self->mError = CTM_INTERNAL_ERROR;
      ok = CTM_FALSE;
  }
  if(!ok)
    return CTM_FALSE;

  // Reset the buffer pointer ("seek" back to the start)
  self->mV5Compat.mCurrentChunk = self->mV5Compat.mFirstChunk;
  self->mV5Compat.mChunkPos = 0;

  // Re-assign the file loader routine so that it will read the converted
  // memory buffer.
  if(self->mFileStream)
  {
    fclose(self->mFileStream);
    self->mFileStream = (FILE *) 0;
  }
  self->mReadFn = _ctmMemChunkRead;
  self->mUserData = (void *) &self->mV5Compat;

  return CTM_TRUE;
}

//-----------------------------------------------------------------------------
// Cleanup v5 specific data from the context.
//-----------------------------------------------------------------------------
void _ctmCleanupV5Data(_CTMcontext * self)
{
  _CTMchunklist *chunk, *next;

  // Free the chunk list
  chunk = self->mV5Compat.mFirstChunk;
  while(chunk)
  {
    if(chunk->mData)
      free(chunk->mData);
    next = chunk->mNext;
    free((void *) chunk);
    chunk = next;
  }

  // Clear the v5 compatibility object
  memset((void *) &self->mV5Compat, 0, sizeof(_CTMv5compat));
}

#else
  // Dummy code (ISO C does not like empty source files)
  void _ctm_v5compat_dummy(void) {}
#endif // _CTM_SUPPORT_V5_FILES
