//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        openctm.c
// Description: API functions.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "openctm.h"
#include "internal.h"

//-----------------------------------------------------------------------------
// _ctmClearMesh() - Clear the mesh in a CTM context.
//-----------------------------------------------------------------------------
static void _ctmClearMesh(_CTMcontext * self)
{
  if(self->mVertices)
    free(self->mVertices);
  if(self->mIndices)
    free(self->mIndices);
  if(self->mTexCoords)
    free(self->mTexCoords);
  if(self->mNormals)
    free(self->mNormals);
  self->mVertices = (CTMfloat *) 0;
  self->mIndices = (CTMuint *) 0;
  self->mTexCoords = (CTMfloat *) 0;
  self->mNormals = (CTMfloat *) 0;
  self->mVertexCount = 0;
  self->mTriangleCount = 0;
}

//-----------------------------------------------------------------------------
// ctmNewContext()
//-----------------------------------------------------------------------------
CTMcontext ctmNewContext(void)
{
  _CTMcontext * self;

  // Allocate memory for the new structure
  self = (_CTMcontext *) malloc(sizeof(_CTMcontext));

  // Initialize structure (set null pointers and zero array lengths)
  memset(self, 0, sizeof(_CTMcontext));
  self->mError = CTM_NO_ERROR;
  self->mMethod = CTM_METHOD_MG1;
  self->mVertexPrecision = 0.001;

  return (CTMcontext) self;
}

//-----------------------------------------------------------------------------
// ctmFreeContext()
//-----------------------------------------------------------------------------
void ctmFreeContext(CTMcontext aContext)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return;

  // Free all resources
  if(self->mVertices)
    free(self->mVertices);
  if(self->mIndices)
    free(self->mIndices);
  if(self->mTexCoords)
    free(self->mTexCoords);
  if(self->mNormals)
    free(self->mNormals);
  if(self->mFileComment)
    free(self->mFileComment);

  // Free the context
  free(self);
}

//-----------------------------------------------------------------------------
// ctmError()
//-----------------------------------------------------------------------------
CTMerror ctmError(CTMcontext aContext)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  CTMerror err;

  if(!self) return CTM_INVALID_CONTEXT;

  // Get error code and reset error state
  err = self->mError;
  self->mError = CTM_NO_ERROR;
  return err;
}

//-----------------------------------------------------------------------------
// ctmGetInteger()
//-----------------------------------------------------------------------------
CTMuint ctmGetInteger(CTMcontext aContext, CTMproperty aProperty)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return 0;

  switch(aProperty)
  {
    case CTM_VERTEX_COUNT:
      return self->mVertexCount;

    case CTM_TRIANGLE_COUNT:
      return self->mTriangleCount;

    case CTM_HAS_TEX_COORDS:
      return self->mTexCoords ? CTM_TRUE : CTM_FALSE;

    case CTM_HAS_NORMALS:
      return self->mNormals ? CTM_TRUE : CTM_FALSE;

    default:
      self->mError = CTM_INVALID_ARGUMENT;
  }

  return 0;
}

//-----------------------------------------------------------------------------
// ctmGetString()
//-----------------------------------------------------------------------------
const char * ctmGetString(CTMcontext aContext, CTMproperty aProperty)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return 0;

  switch(aProperty)
  {
    case CTM_FILE_COMMENT:
      return (const char *) self->mFileComment;

    default:
      self->mError = CTM_INVALID_ARGUMENT;
  }

  return (const char *) 0;
}

//-----------------------------------------------------------------------------
// ctmCompressionMethod()
//-----------------------------------------------------------------------------
void ctmCompressionMethod(CTMcontext aContext, CTMmethod aMethod)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return;

  // Check arguments
  if((aMethod != CTM_METHOD_RAW) && (aMethod != CTM_METHOD_MG1) &&
     (aMethod != CTM_METHOD_MG2))
  {
    self->mError = CTM_INVALID_ARGUMENT;
    return;
  }

  // Set method
  self->mMethod = aMethod;
}

//-----------------------------------------------------------------------------
// ctmVertexPrecision()
//-----------------------------------------------------------------------------
void ctmVertexPrecision(CTMcontext aContext, CTMfloat aPrecision)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return;

  // Check arguments
  if(aPrecision == 0.0)
  {
    self->mError = CTM_INVALID_ARGUMENT;
    return;
  }

  // Set method
  self->mVertexPrecision = aPrecision;
}

//-----------------------------------------------------------------------------
// ctmFileComment()
//-----------------------------------------------------------------------------
void ctmFileComment(CTMcontext aContext, const char * aFileComment)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  int len;
  if(!self) return;

  // Free the old comment string, if necessary
  if(self->mFileComment)
  {
    free(self->mFileComment);
    self->mFileComment = (char *) 0;
  }

  // Get length of string (if empty, do nothing)
  if(!aFileComment)
    return;
  len = strlen(aFileComment);
  if(!len)
    return;

  // Copy the string
  self->mFileComment = (char *) malloc(len + 1);
  if(!self->mFileComment)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return;
  }
  strcpy(self->mFileComment, aFileComment);
}

//-----------------------------------------------------------------------------
// ctmDefineMesh()
//-----------------------------------------------------------------------------
void ctmDefineMesh(CTMcontext aContext, const CTMfloat * aVertices,
                   CTMuint aVertexCount, const CTMuint * aIndices,
                   CTMuint aTriangleCount, const CTMfloat * aTexCoords,
                   const CTMfloat * aNormals)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return;

  // Check arguments
  if(!aVertices || !aIndices || !aVertexCount || !aTriangleCount)
  {
    self->mError = CTM_INVALID_ARGUMENT;
    return;
  }

  // Clear the old mesh, if any
  _ctmClearMesh(self);

  // Copy vertices
  self->mVertices = (CTMfloat *) malloc(aVertexCount * sizeof(CTMfloat) * 3);
  if(!self->mVertices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return;
  }
  memcpy(self->mVertices, aVertices, aVertexCount * sizeof(CTMfloat) * 3);
  self->mVertexCount = aVertexCount;

  // Copy indices
  self->mIndices = (CTMuint *) malloc(aTriangleCount * sizeof(CTMuint) * 3);
  if(!self->mIndices)
  {
    _ctmClearMesh(self);
    self->mError = CTM_OUT_OF_MEMORY;
    return;
  }
  memcpy(self->mIndices, aIndices, aTriangleCount * sizeof(CTMuint) * 3);
  self->mTriangleCount = aTriangleCount;

  // Copy texture coordinates
  if(aTexCoords)
  {
    self->mTexCoords = (CTMfloat *) malloc(aVertexCount * sizeof(CTMfloat) * 2);
    if(!self->mTexCoords)
    {
      _ctmClearMesh(self);
      self->mError = CTM_OUT_OF_MEMORY;
      return;
    }
    memcpy(self->mTexCoords, aTexCoords, aVertexCount * sizeof(CTMfloat) * 2);
  }

  // Copy normals
  if(aNormals)
  {
    self->mNormals = (CTMfloat *) malloc(aVertexCount * sizeof(CTMfloat) * 3);
    if(!self->mNormals)
    {
      _ctmClearMesh(self);
      self->mError = CTM_OUT_OF_MEMORY;
      return;
    }
    memcpy(self->mNormals, aNormals, aVertexCount * sizeof(CTMfloat) * 3);
  }
}

//-----------------------------------------------------------------------------
// ctmGetMesh()
//-----------------------------------------------------------------------------
void ctmGetMesh(CTMcontext aContext, CTMfloat * aVertices,
                CTMuint aVertexCount, CTMuint * aIndices,
                CTMuint aTriangleCount, CTMfloat * aTexCoords,
                CTMfloat * aNormals)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  if(!self) return;

  // Check arguments
  if(!aVertices || !aIndices || (aVertexCount > self->mVertexCount) ||
     (aTriangleCount > self->mTriangleCount))
  {
    self->mError = CTM_INVALID_ARGUMENT;
    return;
  }

  // Copy vertices
  memcpy(aVertices, self->mVertices, aVertexCount * sizeof(CTMfloat) * 3);

  // Copy indices
  memcpy(aIndices, self->mIndices, aTriangleCount * sizeof(CTMuint) * 3);

  // Copy (or clear) texture coordinates
  if(aTexCoords)
  {
    if(self->mTexCoords)
      memcpy(aTexCoords, self->mTexCoords, aVertexCount * sizeof(CTMfloat) * 2);
    else
      memset(aTexCoords, 0, aVertexCount * sizeof(CTMfloat) * 2);
  }

  // Copy (or clear) normals
  if(aNormals)
  {
    if(self->mNormals)
      memcpy(aNormals, self->mNormals, aVertexCount * sizeof(CTMfloat) * 3);
    else
      memset(aNormals, 0, aVertexCount * sizeof(CTMfloat) * 3);
  }
}

//-----------------------------------------------------------------------------
// _ctmDefaultRead()
//-----------------------------------------------------------------------------
static CTMuint _ctmDefaultRead(void * aBuf, CTMuint aCount, void * aUserData)
{
  return (CTMuint) fread(aBuf, 1, (size_t) aCount, (FILE *) aUserData);
}

//-----------------------------------------------------------------------------
// ctmLoad()
//-----------------------------------------------------------------------------
void ctmLoad(CTMcontext aContext, const char * aFileName)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  FILE * f;
  if(!self) return;

  // Open file stream
  f = fopen(aFileName, "rb");
  if(!f)
  {
    self->mError = CTM_FILE_ERROR;
    return;
  }

  // Load the file
  ctmLoadCustom(self, _ctmDefaultRead, (void *) f);

  // Close file stream
  fclose(f);
}

//-----------------------------------------------------------------------------
// ctmLoadCustom()
//-----------------------------------------------------------------------------
void ctmLoadCustom(CTMcontext aContext, CTMreadfn aReadFn, void * aUserData)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  CTMuint formatVersion, flags, method;
  if(!self) return;

  // Initialize stream
  self->mReadFn = aReadFn;
  self->mUserData = aUserData;

  // Clear any old mesh arrays
  _ctmClearMesh(self);

  // Read header from stream
  if(_ctmStreamReadUINT(self) != FOURCC("OCTM"))
  {
    self->mError = CTM_FORMAT_ERROR;
    return;
  }
  formatVersion = _ctmStreamReadUINT(self);
  if((formatVersion < 0x0100) || (formatVersion > CTM_FORMAT_VERSION))
  {
    self->mError = CTM_FORMAT_ERROR;
    return;
  }
  method = _ctmStreamReadUINT(self);
  if(method == FOURCC("RAW\0"))
    self->mMethod = CTM_METHOD_RAW;
  else if(method == FOURCC("MG1\0"))
    self->mMethod = CTM_METHOD_MG1;
  else if(method == FOURCC("MG2\0"))
    self->mMethod = CTM_METHOD_MG2;
  else
  {
    self->mError = CTM_FORMAT_ERROR;
    return;
  }
  self->mVertexCount = _ctmStreamReadUINT(self);
  if(self->mVertexCount == 0)
  {
    self->mError = CTM_FORMAT_ERROR;
    return;
  }
  self->mTriangleCount = _ctmStreamReadUINT(self);
  if(self->mTriangleCount == 0)
  {
    self->mError = CTM_FORMAT_ERROR;
    return;
  }
  flags = _ctmStreamReadUINT(self);
  _ctmStreamReadSTRING(self, &self->mFileComment);

  // Allocate memory for the mesh arrays
  self->mVertices = (CTMfloat *) malloc(self->mVertexCount * sizeof(CTMfloat) * 3);
  if(!self->mVertices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return;
  }
  self->mIndices = (CTMuint *) malloc(self->mTriangleCount * sizeof(CTMuint) * 3);
  if(!self->mIndices)
  {
    _ctmClearMesh(self);
    self->mError = CTM_OUT_OF_MEMORY;
    return;
  }
  if(flags & _CTM_HAS_TEXCOORDS_BIT)
  {
    self->mTexCoords = (CTMfloat *) malloc(self->mVertexCount * sizeof(CTMfloat) * 2);
    if(!self->mTexCoords)
    {
      _ctmClearMesh(self);
      self->mError = CTM_OUT_OF_MEMORY;
      return;
    }
  }
  if(flags & _CTM_HAS_NORMALS_BIT)
  {
    self->mNormals = (CTMfloat *) malloc(self->mVertexCount * sizeof(CTMfloat) * 3);
    if(!self->mNormals)
    {
      _ctmClearMesh(self);
      self->mError = CTM_OUT_OF_MEMORY;
      return;
    }
  }

  // Uncompress from stream
  switch(self->mMethod)
  {
    case CTM_METHOD_RAW:
      _ctmUncompressMesh_RAW(self);
      break;

    case CTM_METHOD_MG1:
      _ctmUncompressMesh_MG1(self);
      break;

    case CTM_METHOD_MG2:
      _ctmUncompressMesh_MG2(self);
      break;
  }
}

//-----------------------------------------------------------------------------
// _ctmDefaultWrite()
//-----------------------------------------------------------------------------
static CTMuint _ctmDefaultWrite(const void * aBuf, CTMuint aCount,
  void * aUserData)
{
  return (CTMuint) fwrite(aBuf, 1, (size_t) aCount, (FILE *) aUserData);
}

//-----------------------------------------------------------------------------
// ctmSave()
//-----------------------------------------------------------------------------
void ctmSave(CTMcontext aContext, const char * aFileName)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  FILE * f;
  if(!self) return;

  // Open file stream
  f = fopen(aFileName, "wb");
  if(!f)
  {
    self->mError = CTM_FILE_ERROR;
    return;
  }

  // Save the file
  ctmSaveCustom(self, _ctmDefaultWrite, (void *) f);

  // Close file stream
  fclose(f);
}

//-----------------------------------------------------------------------------
// ctmSaveCustom()
//-----------------------------------------------------------------------------
void ctmSaveCustom(CTMcontext aContext, CTMwritefn aWriteFn, void * aUserData)
{
  _CTMcontext * self = (_CTMcontext *) aContext;
  CTMuint flags;
  if(!self) return;

  // Check mesh integrity
  if(!self->mVertices || !self->mIndices || (self->mVertexCount < 1) ||
     (self->mTriangleCount < 1))
  {
    self->mError = CTM_INVALID_MESH;
    return;
  }

  // Initialize stream
  self->mWriteFn = aWriteFn;
  self->mUserData = aUserData;

  // Determine flags
  flags = 0;
  if(self->mTexCoords)
    flags |= _CTM_HAS_TEXCOORDS_BIT;
  if(self->mNormals)
    flags |= _CTM_HAS_NORMALS_BIT;

  // Write header to stream
  _ctmStreamWrite(self, (void *) "OCTM", 4);
  _ctmStreamWriteUINT(self, CTM_FORMAT_VERSION);
  switch(self->mMethod)
  {
    case CTM_METHOD_RAW:
      _ctmStreamWrite(self, (void *) "RAW\0", 4);
      break;
    case CTM_METHOD_MG1:
      _ctmStreamWrite(self, (void *) "MG1\0", 4);
      break;
    case CTM_METHOD_MG2:
      _ctmStreamWrite(self, (void *) "MG2\0", 4);
      break;
  }
  _ctmStreamWriteUINT(self, self->mVertexCount);
  _ctmStreamWriteUINT(self, self->mTriangleCount);
  _ctmStreamWriteUINT(self, flags);
  _ctmStreamWriteSTRING(self, self->mFileComment);

  // Compress to stream
  switch(self->mMethod)
  {
    case CTM_METHOD_RAW:
      _ctmCompressMesh_RAW(self);
      break;

    case CTM_METHOD_MG1:
      _ctmCompressMesh_MG1(self);
      break;

    case CTM_METHOD_MG2:
      _ctmCompressMesh_MG2(self);
      break;
  }
}
