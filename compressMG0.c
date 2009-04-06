//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        compressMG0.c
// Description: Implementation of the MG0 compression method.
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

#include "openctm.h"
#include "internal.h"

//-----------------------------------------------------------------------------
// _ctmCompressMesh_MG0() - Compress the mesh that is stored in the CTM
// context using the MG0 method, and write it the the output stream in the CTM
// context.
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG0(_CTMcontext * self)
{
  CTMuint i;

  // Write vertices
  _ctmStreamWrite(self, (void *) "VERT", 4);
  for(i = 0; i < self->mVertexCount * 3; ++ i)
    _ctmStreamWriteFLOAT(self, self->mVertices[i]);

  // Write triangle indices
  _ctmStreamWrite(self, (void *) "INDX", 4);
  for(i = 0; i < self->mTriangleCount * 3; ++ i)
    _ctmStreamWriteUINT(self, self->mIndices[i]);

  // Write texture coordintes
  if(self->mTexCoords)
  {
    _ctmStreamWrite(self, (void *) "TEXC", 4);
    for(i = 0; i < self->mVertexCount * 2; ++ i)
      _ctmStreamWriteFLOAT(self, self->mTexCoords[i]);
  }

  // Write normals
  if(self->mNormals)
  {
    _ctmStreamWrite(self, (void *) "NORM", 4);
    for(i = 0; i < self->mVertexCount * 3; ++ i)
      _ctmStreamWriteFLOAT(self, self->mNormals[i]);
  }

  return 1;
}

//-----------------------------------------------------------------------------
// _ctmUncompressMesh_MG0() - Uncmpress the mesh from the input stream in the
// CTM context using the MG0 method, and store the resulting mesh in the CTM
// context.
//-----------------------------------------------------------------------------
int _ctmUncompressMesh_MG0(_CTMcontext * self)
{
  CTMuint i;

  // Read vertices
  if(_ctmStreamReadUINT(self) != FOURCC("VERT"))
  {
    self->mError = CTM_FORMAT_ERROR;
    return 0;
  }
  for(i = 0; i < self->mVertexCount * 3; ++ i)
    self->mVertices[i] = _ctmStreamReadFLOAT(self);

  // Read triangle indices
  if(_ctmStreamReadUINT(self) != FOURCC("INDX"))
  {
    self->mError = CTM_FORMAT_ERROR;
    return 0;
  }
  for(i = 0; i < self->mTriangleCount * 3; ++ i)
    self->mIndices[i] = _ctmStreamReadUINT(self);

  // Read texture coordintes
  if(self->mTexCoords)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("TEXC"))
    {
      self->mError = CTM_FORMAT_ERROR;
      return 0;
    }
    for(i = 0; i < self->mVertexCount * 2; ++ i)
      self->mTexCoords[i] = _ctmStreamReadFLOAT(self);
  }

  // Read normals
  if(self->mNormals)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("NORM"))
    {
      self->mError = CTM_FORMAT_ERROR;
      return 0;
    }
    for(i = 0; i < self->mVertexCount * 3; ++ i)
      self->mNormals[i] = _ctmStreamReadFLOAT(self);
  }

  return 1;
}
