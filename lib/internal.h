//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        internal.h
// Description: Internal (private) declarations, types and function prototypes.
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

#ifndef __OPENCTM_INTERNAL_H_
#define __OPENCTM_INTERNAL_H_

#include <stdio.h>
#include "config.h"

#ifdef _CTM_SUPPORT_V5_FILES
  #include "v5compat.h"
#endif

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
// OpenCTM file format version (v6).
#define _CTM_FORMAT_VERSION  0x00000006

// Flags for the Mesh flags field of the file header
#define _CTM_HAS_NORMALS_BIT 0x00000001

//-----------------------------------------------------------------------------
// _CTMarray - Internal representation of a typed array.
//-----------------------------------------------------------------------------
typedef struct _CTMarray_struct _CTMarray;
struct _CTMarray_struct {
  void * mData;         // Pointer to the first element of the array
  CTMenum mType;        // Data type
  CTMuint mSize;        // Data size (number of components per element)
  CTMuint mStride;      // Byte offset from one element to the next
};

//-----------------------------------------------------------------------------
// _CTMfloatmap - Internal representation of a floating point based vertex map
// (used for UV maps and attribute maps).
//-----------------------------------------------------------------------------
typedef struct _CTMfloatmap_struct _CTMfloatmap;
struct _CTMfloatmap_struct {
  char * mName;         // Unique name
  char * mFileName;     // File name reference (used only for UV maps)
  CTMfloat mPrecision;  // Precision for this map
  _CTMarray mArray;     // Array reference to callers memory
  _CTMfloatmap * mNext; // Pointer to the next map in the list (linked list)
};

//-----------------------------------------------------------------------------
// _CTMcontext - Internal CTM context structure.
//-----------------------------------------------------------------------------
typedef struct {
  // Context mode (import or export)
  CTMenum mMode;

  // Animation frame count
  CTMint mFrameCount;

  // Current animation frame (zero index)
  CTMint mCurrentFrame;

  // Indices
  _CTMarray mIndices;
  CTMuint mTriangleCount;

  // Vertices
  _CTMarray mVertices;
  CTMuint mVertexCount;

  // Normals (optional)
  _CTMarray mNormals;
  CTMuint mHasNormals;

  // Multiple sets of UV coordinate maps (optional)
  CTMuint mUVMapCount;
  _CTMfloatmap * mUVMaps;

  // Multiple sets of custom vertex attribute maps (optional)
  CTMuint mAttribMapCount;
  _CTMfloatmap * mAttribMaps;

  // Last error code
  CTMenum mError;

  // The selected compression method
  CTMenum mMethod;

  // The selected compression level
  CTMuint mCompressionLevel;

  // Vertex coordinate precision
  CTMfloat mVertexPrecision;

  // Normal precision (angular + magnitude)
  CTMfloat mNormalPrecision;

  // File comment
  char * mFileComment;

  // Read() function pointer
  CTMreadfn mReadFn;

  // Write() function pointer
  CTMwritefn mWriteFn;

  // User data (for stream read/write - usually the stream handle)
  void * mUserData;

  // If we use our own file handle, set this handle (otherwise nil)
  FILE * mFileStream;

#ifdef _CTM_SUPPORT_V5_FILES
  // v5 compatibility data
  _CTMv5compat * mV5Compat;
#endif
} _CTMcontext;

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Generate a FourCC 32-bit unsigned integer from a string.
#define FOURCC(str) (((CTMuint) str[0]) | (((CTMuint) str[1]) << 8) | \
                    (((CTMuint) str[2]) << 16) | (((CTMuint) str[3]) << 24))

// Use this macro to avoid compiler warnings for unused function parameters.
#define DUMMYUSE(x) (void) (x)

//-----------------------------------------------------------------------------
// Function prototypes for array.c
//-----------------------------------------------------------------------------
CTMuint _ctmGetArrayi(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent);
CTMfloat _ctmGetArrayf(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent);
void _ctmSetArrayi(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent, CTMuint aValue);
void _ctmSetArrayf(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent, CTMfloat aValue);

//-----------------------------------------------------------------------------
// Function prototypes for stream.c
//-----------------------------------------------------------------------------
CTMuint _ctmStreamRead(_CTMcontext * self, void * aBuf, CTMuint aCount);
CTMuint _ctmStreamWrite(_CTMcontext * self, void * aBuf, CTMuint aCount);
CTMuint _ctmStreamReadUINT(_CTMcontext * self);
void _ctmStreamWriteUINT(_CTMcontext * self, CTMuint aValue);
CTMfloat _ctmStreamReadFLOAT(_CTMcontext * self);
void _ctmStreamWriteFLOAT(_CTMcontext * self, CTMfloat aValue);
void _ctmStreamReadSTRING(_CTMcontext * self, char ** aValue);
void _ctmStreamWriteSTRING(_CTMcontext * self, const char * aValue);
int _ctmStreamReadPackedInts(_CTMcontext * self, CTMint * aData, CTMuint aCount, CTMuint aSize, CTMint aSignedInts);
int _ctmStreamWritePackedInts(_CTMcontext * self, CTMint * aData, CTMuint aCount, CTMuint aSize, CTMint aSignedInts);
int _ctmStreamReadPackedFloatArray(_CTMcontext * self, _CTMarray * aArray, CTMuint aCount, CTMuint aSize);
int _ctmStreamWritePackedFloatArray(_CTMcontext * self, _CTMarray * aArray, CTMuint aCount, CTMuint aSize);

//-----------------------------------------------------------------------------
// Function prototypes for compressRAW.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_RAW(_CTMcontext * self);
int _ctmUncompressMesh_RAW(_CTMcontext * self);
int _ctmCompressFrame_RAW(_CTMcontext * self);
int _ctmUncompressFrame_RAW(_CTMcontext * self);

//-----------------------------------------------------------------------------
// Function prototypes for compressMG1.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG1(_CTMcontext * self);
int _ctmUncompressMesh_MG1(_CTMcontext * self);
int _ctmCompressFrame_MG1(_CTMcontext * self);
int _ctmUncompressFrame_MG1(_CTMcontext * self);

//-----------------------------------------------------------------------------
// Function prototypes for compressMG2.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG2(_CTMcontext * self);
int _ctmUncompressMesh_MG2(_CTMcontext * self);
int _ctmCompressFrame_MG2(_CTMcontext * self);
int _ctmUncompressFrame_MG2(_CTMcontext * self);

//-----------------------------------------------------------------------------
// Function prototypes for v5compat.c
//-----------------------------------------------------------------------------
#ifdef _CTM_SUPPORT_V5_FILES
int _ctmLoadV5FileToMem(_CTMcontext * self);
void _ctmCleanupV5Data(_CTMcontext * self);
#endif

#endif // __OPENCTM_INTERNAL_H_
