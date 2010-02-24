//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        compressRAW.c
// Description: Implementation of the RAW compression method.
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

#include "openctm2.h"
#include "internal.h"

#ifdef _CTM_SUPPORT_RAW

#ifdef __DEBUG_
#include <stdio.h>
#endif


#ifdef _CTM_SUPPORT_SAVE
//-----------------------------------------------------------------------------
// _ctmCompressMesh_RAW() - Compress the mesh that is stored in the CTM
// context using the RAW method, and write it the the output stream in the CTM
// context.
//-----------------------------------------------------------------------------
int _ctmCompressMesh_RAW(_CTMcontext * self)
{
  CTMuint i, j;
  _CTMfloatmap * map;

#ifdef __DEBUG_
  printf("COMPRESSION METHOD: RAW\n");
#endif

  // Write triangle indices
#ifdef __DEBUG_
  printf("Inidices: %d bytes\n", (CTMuint)(self->mTriangleCount * 3 * sizeof(CTMuint)));
#endif
  _ctmStreamWrite(self, (void *) "INDX", 4);
  for(i = 0; i < self->mTriangleCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmStreamWriteUINT(self, _ctmGetArrayi(&self->mIndices, i, j));

  // Write vertices
#ifdef __DEBUG_
  printf("Vertices: %d bytes\n", (CTMuint)(self->mVertexCount * 3 * sizeof(CTMfloat)));
#endif
  _ctmStreamWrite(self, (void *) "VERT", 4);
  for(i = 0; i < self->mVertexCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&self->mVertices, i, j));

  // Write normals
  if(self->mHasNormals)
  {
#ifdef __DEBUG_
    printf("Normals: %d bytes\n", (CTMuint)(self->mVertexCount * 3 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "NORM", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 3; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&self->mNormals, i, j));
  }

  // Write UV maps
  map = self->mUVMaps;
  while(map)
  {
#ifdef __DEBUG_
    printf("UV coordinates (%s): %d bytes\n", map->mName ? map->mName : "no name", (CTMuint)(self->mVertexCount * 2 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "TEXC", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 2; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&map->mArray, i, j));
    map = map->mNext;
  }

  // Write attribute maps
  map = self->mAttribMaps;
  while(map)
  {
#ifdef __DEBUG_
    printf("Vertex attributes (%s): %d bytes\n", map->mName ? map->mName : "no name", (CTMuint)(self->mVertexCount * 4 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "ATTR", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 4; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&map->mArray, i, j));
    map = map->mNext;
  }

  return CTM_TRUE;
}
#endif

#ifdef _CTM_SUPPORT_SAVE
//-----------------------------------------------------------------------------
// _ctmCompressFrame_RAW() - Compress the next frame that is stored in the CTM
// context using the RAW method, and write it the the output stream in the CTM
// context.
//-----------------------------------------------------------------------------
int _ctmCompressFrame_RAW(_CTMcontext * self)
{
  CTMuint i, j;
  _CTMfloatmap * map;

  // Write vertices
#ifdef __DEBUG_
  printf("Vertices: %d bytes\n", (CTMuint)(self->mVertexCount * 3 * sizeof(CTMfloat)));
#endif
  _ctmStreamWrite(self, (void *) "VERT", 4);
  for(i = 0; i < self->mVertexCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&self->mVertices, i, j));

  // Write normals
  if(self->mHasNormals)
  {
#ifdef __DEBUG_
    printf("Normals: %d bytes\n", (CTMuint)(self->mVertexCount * 3 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "NORM", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 3; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&self->mNormals, i, j));
  }

  // Write UV maps
  map = self->mUVMaps;
  while(map)
  {
#ifdef __DEBUG_
    printf("UV coordinates (%s): %d bytes\n", map->mName ? map->mName : "no name", (CTMuint)(self->mVertexCount * 2 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "TEXC", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 2; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&map->mArray, i, j));
    map = map->mNext;
  }

  // Write attribute maps
  map = self->mAttribMaps;
  while(map)
  {
#ifdef __DEBUG_
    printf("Vertex attributes (%s): %d bytes\n", map->mName ? map->mName : "no name", (CTMuint)(self->mVertexCount * 4 * sizeof(CTMfloat)));
#endif
    _ctmStreamWrite(self, (void *) "ATTR", 4);
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 4; ++ j)
        _ctmStreamWriteFLOAT(self, _ctmGetArrayf(&map->mArray, i, j));
    map = map->mNext;
  }

  return CTM_TRUE;
}
#endif

//-----------------------------------------------------------------------------
// _ctmUncompressMesh_RAW() - Uncompress the mesh from the input stream in the
// CTM context using the RAW method, and store the resulting mesh in the CTM
// context.
//-----------------------------------------------------------------------------
int _ctmUncompressMesh_RAW(_CTMcontext * self)
{
  CTMuint i, j;
  _CTMfloatmap * map;

  // Read triangle indices
  if(_ctmStreamReadUINT(self) != FOURCC("INDX"))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }
  for(i = 0; i < self->mTriangleCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmSetArrayi(&self->mIndices, i, j, _ctmStreamReadUINT(self));

  // Read vertices
  if(_ctmStreamReadUINT(self) != FOURCC("VERT"))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }
  for(i = 0; i < self->mVertexCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmSetArrayf(&self->mVertices, i, j, _ctmStreamReadFLOAT(self));

  // Read normals
  if(self->mHasNormals)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("NORM"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 3; ++ j)
        _ctmSetArrayf(&self->mNormals, i, j, _ctmStreamReadFLOAT(self));
  }

  // Read UV maps
  map = self->mUVMaps;
  while(map)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("TEXC"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 2; ++ j)
        _ctmSetArrayf(&map->mArray, i, j, _ctmStreamReadFLOAT(self));
    map = map->mNext;
  }

  // Read attribute maps
  map = self->mAttribMaps;
  while(map)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("ATTR"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 4; ++ j)
        _ctmSetArrayf(&map->mArray, i, j, _ctmStreamReadFLOAT(self));
    map = map->mNext;
  }

  return CTM_TRUE;
}

//-----------------------------------------------------------------------------
// _ctmUncompressFrame_RAW() - Uncompress the next frame from the input stream
// in the CTM context using the RAW method, and store the resulting mesh in the
// CTM context.
//-----------------------------------------------------------------------------
int _ctmUncompressFrame_RAW(_CTMcontext * self)
{
  CTMuint i, j;
  _CTMfloatmap * map;

  // Read vertices
  if(_ctmStreamReadUINT(self) != FOURCC("VERT"))
  {
    self->mError = CTM_BAD_FORMAT;
    return CTM_FALSE;
  }
  for(i = 0; i < self->mVertexCount; ++ i)
    for(j = 0; j < 3; ++ j)
      _ctmSetArrayf(&self->mVertices, i, j, _ctmStreamReadFLOAT(self));

  // Read normals
  if(self->mHasNormals)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("NORM"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 3; ++ j)
        _ctmSetArrayf(&self->mNormals, i, j, _ctmStreamReadFLOAT(self));
  }

  // Read UV maps
  map = self->mUVMaps;
  while(map)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("TEXC"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 2; ++ j)
        _ctmSetArrayf(&map->mArray, i, j, _ctmStreamReadFLOAT(self));
    map = map->mNext;
  }

  // Read attribute maps
  map = self->mAttribMaps;
  while(map)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("ATTR"))
    {
      self->mError = CTM_BAD_FORMAT;
      return CTM_FALSE;
    }
    for(i = 0; i < self->mVertexCount; ++ i)
      for(j = 0; j < 4; ++ j)
        _ctmSetArrayf(&map->mArray, i, j, _ctmStreamReadFLOAT(self));
    map = map->mNext;
  }

  return CTM_TRUE;
}

#endif // _CTM_SUPPORT_RAW
