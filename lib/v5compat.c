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

//-----------------------------------------------------------------------------
// _ctmNewMemChunk() - Allocate memory for a new chunk.
//-----------------------------------------------------------------------------
static _CTMchunklist * _ctmNewMemChunk(_CTMcontext * self, CTMuint aSize)
{
  _CTMchunklist *chunk;

  // Allocate memory for the object
  chunk = (_CTMchunklist *) malloc(sizeof(_CTMchunklist));
  if(!chunk)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return (_CTMchunklist *) 0;
  }

  // Initialize the object
  chunk->mData = (CTMubyte *) malloc(aSize);
  if(!chunk->mData)
  {
    free((void *) chunk);
    self->mError = CTM_OUT_OF_MEMORY;
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
// _ctmSetUINT()
//-----------------------------------------------------------------------------
void _ctmSetUINT(CTMubyte * aBuf, CTMuint aValue)
{
  aBuf[0] = (CTMubyte)aValue;
  aBuf[1] = (CTMubyte)(aValue >> 8);
  aBuf[2] = (CTMubyte)(aValue >> 16);
  aBuf[3] = (CTMubyte)(aValue >> 24);
}

//-----------------------------------------------------------------------------
// _ctmGetUINT()
//-----------------------------------------------------------------------------
CTMuint _ctmGetUINT(CTMubyte * aBuf)
{
  return ((CTMuint)aBuf[0]) |
         (((CTMuint)aBuf[1]) << 8) |
         (((CTMuint)aBuf[2]) << 16) |
         (((CTMuint)aBuf[2]) << 24);
}

//-----------------------------------------------------------------------------
// _ctmMemChunkRead()
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

//-----------------------------------------------------------------------------
// Load a v5 file into memory. While loading the data, convert it to the file
// format version that is supported by the current library.
//-----------------------------------------------------------------------------
int _ctmLoadV5FileToMem(_CTMcontext * self)
{
  _CTMchunklist *chunk, *preUVInfoChunk;
  CTMuint len;

  // Already initialized?
  if(self->mV5Compat)
    _ctmCleanupV5Data(self);

  // Initialize the v5 compatibility object
  self->mV5Compat = (_CTMv5compat *) malloc(sizeof(_CTMv5compat));
  if(!self->mV5Compat)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  memset((void *) self->mV5Compat, 0, sizeof(_CTMv5compat));

  // Load header
  if(!(chunk = _ctmNewMemChunk(self, 28)))
    return CTM_FALSE;
  _ctmAppendMemChunkLast(&self->mV5Compat->mFirstChunk, chunk);
  _ctmStreamRead(self, chunk->mData, 24);
  _ctmSetUINT(&chunk->mData[24], 0); // mFrameCount = 0

  // Load file comment
  len = _ctmStreamReadUINT(self);
  if(!(chunk = _ctmNewMemChunk(self, 4 + len)))
    return CTM_FALSE;
  _ctmAppendMemChunkLast(&self->mV5Compat->mFirstChunk, chunk);
  _ctmSetUINT(&chunk->mData[0], len);
  _ctmStreamRead(self, &chunk->mData[4], len);

  // Here is where we would want to insert UV and attrib map info (if any)...
  preUVInfoChunk = chunk;

  // Load the rest of the file (method dependent)
  // FIXME!

  // Reset the buffer pointer ("seek" back to the start)
  self->mV5Compat->mCurrentChunk = self->mV5Compat->mFirstChunk;
  self->mV5Compat->mChunkPos = 0;

  // Re-assign the file loader routine so that it will read the converted
  // memory buffer.
  if(self->mFileStream)
  {
    fclose(self->mFileStream);
    self->mFileStream = (FILE *) 0;
  }
  self->mReadFn = _ctmMemChunkRead;
  self->mUserData = (void *) self->mV5Compat;

  return CTM_TRUE;
}

//-----------------------------------------------------------------------------
// Cleanup v5 specific data from the context.
//-----------------------------------------------------------------------------
void _ctmCleanupV5Data(_CTMcontext * self)
{
  _CTMchunklist *chunk, *next;

  // Nothing to do?
  if(!self->mV5Compat)
    return;

  // Free the chunk list
  chunk = self->mV5Compat->mFirstChunk;
  while(chunk)
  {
    if(chunk->mData)
      free(chunk->mData);
    next = chunk->mNext;
    free(chunk);
    chunk = next;
  }

  // Free the v5 compat object
  free((void *) self->mV5Compat);
  self->mV5Compat = (_CTMv5compat *) 0;
}

#else
  // Dummy code (ISO C does not like empty source files)
  void _ctm_v5compat_dummy(void) {}
#endif // _CTM_SUPPORT_V5_FILES
