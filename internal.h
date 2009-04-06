//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        internal.h
// Description: Internal (private) declarations, types and function prototypes.
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

#ifndef __OPENCTM_INTERNAL_H_
#define __OPENCTM_INTERNAL_H_

//-----------------------------------------------------------------------------
// _CTMcontext - Internal CTM context structure.
//-----------------------------------------------------------------------------
typedef struct {
  // Vertices
  CTMfloat *mVertices;
  CTMuint  mVertexCount;

  // Indices
  CTMuint  *mIndices;
  CTMuint  mTriangleCount;

  // Texture coordinates (optional)
  CTMfloat *mTexCoords;

  // Normals (optional)
  CTMfloat *mNormals;

  // Last error code
  CTMerror mError;

  // The selected compression method
  CTMmethod mMethod;

  // File comment
  char * mFileComment;

  // Read() function pointer
  CTMreadfn mReadFn;

  // Write() function pointer
  CTMwritefn mWriteFn;

  // User data (for stream read/write - usually the stream handle)
  void * mUserData;
} _CTMcontext;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
// Flags for the Flags field in the OpenCTM file header
#define _CTM_HAS_TEXCOORDS_BIT 0x00000001
#define _CTM_HAS_NORMALS_BIT   0x00000002

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define FOURCC(str) (((CTMuint) str[0]) | (((CTMuint) str[1]) << 8) | \
                    (((CTMuint) str[2]) << 16) | (((CTMuint) str[3]) << 24))

//-----------------------------------------------------------------------------
// Funcion prototypes for stream.c
//-----------------------------------------------------------------------------
CTMuint _ctmStreamRead(_CTMcontext * self, void * aBuf, CTMuint aCount);
CTMuint _ctmStreamWrite(_CTMcontext * self, void * aBuf, CTMuint aCount);
CTMuint _ctmStreamReadUINT(_CTMcontext * self);
void _ctmStreamWriteUINT(_CTMcontext * self, CTMuint aValue);
CTMfloat _ctmStreamReadFLOAT(_CTMcontext * self);
void _ctmStreamWriteFLOAT(_CTMcontext * self, CTMfloat aValue);
void _ctmStreamReadSTRING(_CTMcontext * self, char ** aValue);
void _ctmStreamWriteSTRING(_CTMcontext * self, const char * aValue);
int _ctmStreamReadPackedInts(_CTMcontext * self, CTMint * aData, CTMuint aCount, CTMuint aSize);
int _ctmStreamWritePackedInts(_CTMcontext * self, CTMint * aData, CTMuint aCount, CTMuint aSize);
int _ctmStreamReadPackedFloats(_CTMcontext * self, CTMfloat * aData, CTMuint aCount, CTMuint aSize);
int _ctmStreamWritePackedFloats(_CTMcontext * self, CTMfloat * aData, CTMuint aCount, CTMuint aSize);

//-----------------------------------------------------------------------------
// Funcion prototypes for pack.c
//-----------------------------------------------------------------------------
unsigned char * _ctmPackFloats(CTMfloat * aData, CTMuint aCount, CTMuint * aSize);

//-----------------------------------------------------------------------------
// Funcion prototypes for compressMG0.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG0(_CTMcontext * self);
int _ctmUncompressMesh_MG0(_CTMcontext * self);

//-----------------------------------------------------------------------------
// Funcion prototypes for compressMG1.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG1(_CTMcontext * self);
int _ctmUncompressMesh_MG1(_CTMcontext * self);

//-----------------------------------------------------------------------------
// Funcion prototypes for compressMG2.c
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG2(_CTMcontext * self);
int _ctmUncompressMesh_MG2(_CTMcontext * self);

#endif // __OPENCTM_INTERNAL_H_
