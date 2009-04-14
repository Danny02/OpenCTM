//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        compressMG2.c
// Description: Implementation of the MG2 compression method.
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
#include <math.h>
#include "openctm.h"
#include "internal.h"

#define __DEBUG_
#ifdef __DEBUG_
#include <stdio.h>
#endif

//-----------------------------------------------------------------------------
// _CTMgrid - 3D space subdivision grid.
//-----------------------------------------------------------------------------
typedef struct {
  // Axis-aligned boudning box for the grid.
  CTMfloat mMin[3];
  CTMfloat mMax[3];

  // How many divisions per axis (minimum 1).
  CTMuint mDivision[3];

  // Size of each grid box.
  CTMfloat mSize[3];
} _CTMgrid;

//-----------------------------------------------------------------------------
// _CTMsortvertex - Vertex information.
//-----------------------------------------------------------------------------
typedef struct {
  // Vertex X coordinate (used for sorting).
  CTMfloat x;

  // Grid index. This is the index into the 3D space subdivision grid.
  CTMuint mGridIndex;

  // Original index (before sorting).
  CTMuint mOriginalIndex;
} _CTMsortvertex;

//-----------------------------------------------------------------------------
// _ctmSetupGrid() - Setup the 3D space subdivision grid.
//-----------------------------------------------------------------------------
static void _ctmSetupGrid(_CTMcontext * self, _CTMgrid * aGrid)
{
  CTMuint i;
  CTMfloat factor[3], sum, wantedGrids;

  // Calculate the mesh bounding box
  aGrid->mMin[0] = aGrid->mMax[0] = self->mVertices[0];
  aGrid->mMin[1] = aGrid->mMax[1] = self->mVertices[1];
  aGrid->mMin[2] = aGrid->mMax[2] = self->mVertices[2];
  for(i = 1; i < self->mVertexCount; ++ i)
  {
    if(self->mVertices[i * 3] < aGrid->mMin[0])
      aGrid->mMin[0] = self->mVertices[i * 3];
    else if(self->mVertices[i * 3] > aGrid->mMax[0])
      aGrid->mMax[0] = self->mVertices[i * 3];
    if(self->mVertices[i * 3 + 1] < aGrid->mMin[1])
      aGrid->mMin[1] = self->mVertices[i * 3 + 1];
    else if(self->mVertices[i * 3 + 1] > aGrid->mMax[1])
      aGrid->mMax[1] = self->mVertices[i * 3 + 1];
    if(self->mVertices[i * 3 + 2] < aGrid->mMin[2])
      aGrid->mMin[2] = self->mVertices[i * 3 + 2];
    else if(self->mVertices[i * 3 + 2] > aGrid->mMax[2])
      aGrid->mMax[2] = self->mVertices[i * 3 + 2];
  }

  // Determine optimal grid resolution, based on the number of vertices and
  // the bounding box
  for(i = 0; i < 3; ++ i)
    factor[i] = aGrid->mMax[i] - aGrid->mMin[i];
  sum = factor[0] + factor[1] + factor[2];
  if(sum > 1e-30)
  {
    sum = 1.0 / sum;
    for(i = 0; i < 3; ++ i)
      factor[i] *= sum;
    wantedGrids = pow(100.0 * self->mVertexCount, 1.0 / 3.0);
    for(i = 0; i < 3; ++ i)
    {
      aGrid->mDivision[i] = ceil(wantedGrids * factor[i]);
      if(aGrid->mDivision[i] < 1)
        aGrid->mDivision[i] = 1;
    }
  }
  else
  {
    aGrid->mDivision[0] = 4;
    aGrid->mDivision[1] = 4;
    aGrid->mDivision[2] = 4;
  }
#ifdef __DEBUG_
  printf("Division: (%d %d %d)\n", aGrid->mDivision[0], aGrid->mDivision[1], aGrid->mDivision[2]);
#endif

  // Calculate grid sizes
  for(i = 0; i < 3; ++ i)
    aGrid->mSize[i] = (aGrid->mMax[i] - aGrid->mMin[i]) / aGrid->mDivision[i];
}

//-----------------------------------------------------------------------------
// _ctmPointToGridIdx() - Convert a point to a grid index.
//-----------------------------------------------------------------------------
static CTMuint _ctmPointToGridIdx(_CTMgrid * aGrid, CTMfloat * aPoint)
{
  CTMuint i, idx[3];

  for(i = 0; i < 3; ++ i)
  {
    idx[i] = floor((aPoint[i] - aGrid->mMin[i]) / aGrid->mSize[i]);
    if(idx[i] >= aGrid->mDivision[i])
      idx[i] = aGrid->mDivision[i] - 1;
  }

  return idx[0] + aGrid->mDivision[0] * (idx[1] + aGrid->mDivision[1] * idx[2]);
}

//-----------------------------------------------------------------------------
// _ctmGridIdxToPoint() - Convert a grid index to a point (the min x/y/z for
// the given grid box).
//-----------------------------------------------------------------------------
static void _ctmGridIdxToPoint(_CTMgrid * aGrid, CTMuint aIdx, CTMfloat * aPoint)
{
  CTMuint gridIdx[3], zdiv, ydiv, i;

  zdiv = aGrid->mDivision[0] * aGrid->mDivision[1];
  ydiv = aGrid->mDivision[0];

  gridIdx[2] =  aIdx / zdiv;
  aIdx -= gridIdx[2] * zdiv;
  gridIdx[1] =  aIdx / ydiv;
  aIdx -= gridIdx[1] * ydiv;
  gridIdx[0] = aIdx;

  for(i = 0; i < 3; ++ i)
    aPoint[i] = gridIdx[i] * aGrid->mSize[i] + aGrid->mMin[i];
}

//-----------------------------------------------------------------------------
// _compareVertex() - Comparator for the vertex sorting.
//-----------------------------------------------------------------------------
static int _compareVertex(const void * elem1, const void * elem2)
{
  _CTMsortvertex * v1 = (_CTMsortvertex *) elem1;
  _CTMsortvertex * v2 = (_CTMsortvertex *) elem2;
  if(v1->mGridIndex != v2->mGridIndex)
    return v1->mGridIndex - v2->mGridIndex;
  else if(v1->x < v2->x)
    return -1;
  else if(v1->x > v2->x)
    return 1;
  else
    return 0;
}

//-----------------------------------------------------------------------------
// _ctmSortVertices() - Setup the vertex array. Assign each vertex to a grid
// box, and sort all vertices.
//-----------------------------------------------------------------------------
static void _ctmSortVertices(_CTMcontext * self, _CTMsortvertex * aSortVertices,
  _CTMgrid * aGrid)
{
  CTMuint i;

  // Prepare sort vertex array
  for(i = 0; i < self->mVertexCount; ++ i)
  {
    // Store vertex properties in the sort vertex array
    aSortVertices[i].x = self->mVertices[i * 3];
    aSortVertices[i].mGridIndex = _ctmPointToGridIdx(aGrid, &self->mVertices[i * 3]);
    aSortVertices[i].mOriginalIndex = i;
  }

  // Sort vertices. The elements are first sorted by their grid indices, and
  // scondly by their x coordinates.
  qsort((void *) aSortVertices, self->mVertexCount, sizeof(_CTMsortvertex), _compareVertex);
}

//-----------------------------------------------------------------------------
// _ctmReIndexIndices() - Re-index all indices, based on the sorted vertices.
//-----------------------------------------------------------------------------
static int _ctmReIndexIndices(_CTMcontext * self, _CTMsortvertex * aSortVertices,
  CTMuint * aIndices)
{
  CTMuint i, * indexLUT;

  // Create temporary lookup-array, O(n)
  indexLUT = (CTMuint *) malloc(sizeof(CTMuint) * self->mVertexCount);
  if(!indexLUT)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return 0;
  }
  for(i = 0; i < self->mVertexCount; ++ i)
    indexLUT[aSortVertices[i].mOriginalIndex] = i;

  // Convert old indices to new indices, O(n)
  for(i = 0; i < self->mTriangleCount * 3; ++ i)
    aIndices[i] = indexLUT[self->mIndices[i]];

  // Free temporary lookup-array
  free((void *) indexLUT);

  return 1;
}

//-----------------------------------------------------------------------------
// _compareTriangle() - Comparator for the triangle sorting.
//-----------------------------------------------------------------------------
static int _compareTriangle(const void * elem1, const void * elem2)
{
  CTMuint * tri1 = (CTMuint *) elem1;
  CTMuint * tri2 = (CTMuint *) elem2;
  if(tri1[0] != tri2[0])
    return tri1[0] - tri2[0];
  else
    return tri1[1] - tri2[1];
}

//-----------------------------------------------------------------------------
// _ctmReArrangeTriangles() - Re-arrange all triangles for optimal
// compression.
//-----------------------------------------------------------------------------
static void _ctmReArrangeTriangles(_CTMcontext * self, CTMuint * aIndices)
{
  CTMuint * tri, tmp, i;

  // Step 1: Make sure that the first index of each triangle is the smallest
  // one (rotate triangle nodes if necessary)
  for(i = 0; i < self->mTriangleCount; ++ i)
  {
    tri = &aIndices[i * 3];
    if((tri[1] < tri[0]) && (tri[1] < tri[2]))
    {
      tmp = tri[0];
      tri[0] = tri[1];
      tri[1] = tri[2];
      tri[2] = tmp;
    }
    else if((tri[2] < tri[0]) && (tri[2] < tri[1]))
    {
      tmp = tri[0];
      tri[0] = tri[2];
      tri[2] = tri[1];
      tri[1] = tmp;
    }
  }

  // Step 2: Sort the triangles based on the first triangle index
  qsort((void *) aIndices, self->mTriangleCount, sizeof(CTMuint) * 3, _compareTriangle);
}

//-----------------------------------------------------------------------------
// _ctmMakeIndexDeltas() - Calculate various forms of derivatives in order to
// reduce data entropy.
//-----------------------------------------------------------------------------
static void _ctmMakeIndexDeltas(_CTMcontext * self, CTMuint * aIndices)
{
  CTMint i;
  for(i = self->mTriangleCount - 1; i >= 0; -- i)
  {
    // Step 1: Calculate delta from second triangle index to the previous
    // second triangle index, if the previous triangle shares the same first
    // index, otherwise calculate the delta to the first triangle index
    if((i >= 1) && (aIndices[i * 3] == aIndices[(i - 1) * 3]))
      aIndices[i * 3 + 1] -= aIndices[(i - 1) * 3 + 1];
    else
      aIndices[i * 3 + 1] -= aIndices[i * 3];

    // Step 2: Calculate delta from third triangle index to the first triangle
    // index
    aIndices[i * 3 + 2] -= aIndices[i * 3];

    // Step 3: Calculate derivative of the first triangle index
    if(i >= 1)
      aIndices[i * 3] -= aIndices[(i - 1) * 3];
  }
}

//-----------------------------------------------------------------------------
// _ctmRestoreIndices() - Restore original indices (inverse derivative
// operation).
//-----------------------------------------------------------------------------
static void _ctmRestoreIndices(_CTMcontext * self, CTMuint * aIndices)
{
  CTMuint i;

  for(i = 0; i < self->mTriangleCount; ++ i)
  {
    // Step 1: Reverse derivative of the first triangle index
    if(i >= 1)
      aIndices[i * 3] += aIndices[(i - 1) * 3];

    // Step 2: Reverse delta from third triangle index to the first triangle
    // index
    aIndices[i * 3 + 2] += aIndices[i * 3];

    // Step 3: Reverse delta from second triangle index to the previous
    // second triangle index, if the previous triangle shares the same first
    // index, otherwise reverse the delta to the first triangle index
    if((i >= 1) && (aIndices[i * 3] == aIndices[(i - 1) * 3]))
      aIndices[i * 3 + 1] += aIndices[(i - 1) * 3 + 1];
    else
      aIndices[i * 3 + 1] += aIndices[i * 3];
  }
}

//-----------------------------------------------------------------------------
// _ctmMakeVertexDeltas() - Calculate various forms of derivatives in order to
// reduce data entropy.
//-----------------------------------------------------------------------------
static void _ctmMakeVertexDeltas(_CTMcontext * self, CTMint * aIntVertices,
  _CTMsortvertex * aSortVertices, _CTMgrid * aGrid)
{
  CTMuint i, gridIdx, prevGridIndex, oldIdx;
  CTMfloat gridOrigin[3], scale;
  CTMint deltaX, prevDeltaX;

  // Vertex scaling factor
  scale = 1.0 / self->mVertexPrecision;

  prevGridIndex = 0x7fffffff;
  prevDeltaX = 0;
  for(i = 0; i < self->mVertexCount; ++ i)
  {
    // Get grid box origin
    gridIdx = aSortVertices[i].mGridIndex;
    _ctmGridIdxToPoint(aGrid, gridIdx, gridOrigin);

    // Get old vertex coordinate index (before vertex sorting)
    oldIdx = aSortVertices[i].mOriginalIndex;

    // Store delta to the grid box origin in the integer vertex array. For the
    // X axis (which is sorted) we also do the delta to the previous coordinate
    // in the box.
    deltaX = floor(scale * (self->mVertices[oldIdx * 3] - gridOrigin[0]) + 0.5);
    if(gridIdx == prevGridIndex)
      aIntVertices[i * 3] = deltaX - prevDeltaX;
    else
      aIntVertices[i * 3] = deltaX;
    aIntVertices[i * 3 + 1] = floor(scale * (self->mVertices[oldIdx * 3 + 1] - gridOrigin[1]) + 0.5);
    aIntVertices[i * 3 + 2] = floor(scale * (self->mVertices[oldIdx * 3 + 2] - gridOrigin[2]) + 0.5);

    prevGridIndex = gridIdx;
    prevDeltaX = deltaX;
  }
}

//-----------------------------------------------------------------------------
// _ctmRestoreVertices() - Calculate inverse derivatives of the vertices.
//-----------------------------------------------------------------------------
static void _ctmRestoreVertices(_CTMcontext * self, CTMint * aIntVertices,
  CTMuint * aGridIndices, _CTMgrid * aGrid)
{
  CTMuint i, gridIdx, prevGridIndex;
  CTMfloat gridOrigin[3], scale;
  CTMint deltaX, prevDeltaX;

  scale = self->mVertexPrecision;

  prevGridIndex = 0x7fffffff;
  prevDeltaX = 0;
  for(i = 0; i < self->mVertexCount; ++ i)
  {
    // Get grid box origin
    gridIdx = aGridIndices[i];
    _ctmGridIdxToPoint(aGrid, gridIdx, gridOrigin);

    // Restore original point
    deltaX = aIntVertices[i * 3];
    if(gridIdx == prevGridIndex)
      deltaX += prevDeltaX;
    self->mVertices[i * 3] = scale * deltaX + gridOrigin[0];
    self->mVertices[i * 3 + 1] = scale * aIntVertices[i * 3 + 1] + gridOrigin[1];
    self->mVertices[i * 3 + 2] = scale * aIntVertices[i * 3 + 2] + gridOrigin[2];

    prevGridIndex = gridIdx;
    prevDeltaX = deltaX;
  }
}

//-----------------------------------------------------------------------------
// _ctmMakeTexCoordDeltas() - Calculate various forms of derivatives in order
// to reduce data entropy.
//-----------------------------------------------------------------------------
static void _ctmMakeTexCoordDeltas(_CTMcontext * self, CTMint * aIntTexCoords,
  _CTMsortvertex * aSortVertices)
{
 CTMuint i, gridIdx, prevGridIdx, oldIdx;
 CTMint u, v, prevU, prevV;
 CTMfloat scale;

  // Texture coordinate scaling factor
  scale = 1.0 / self->mTexCoordPrecision;

  prevGridIdx = 0x7fffffff;
  prevU = prevV = 0;
  for(i = 0; i < self->mVertexCount; ++ i)
  {
    // Get grid box index
    gridIdx = aSortVertices[i].mGridIndex;

    // Get old texture coordinate index (before vertex sorting)
    oldIdx = aSortVertices[i].mOriginalIndex;

    // Convert to fixed point
    u = floor(scale * self->mTexCoords[oldIdx * 2] + 0.5);
    v = floor(scale * self->mTexCoords[oldIdx * 2 + 1] + 0.5);

    // Calculate delta (when feasible) and store it in the converted array
    if(gridIdx == prevGridIdx)
    {
      aIntTexCoords[i * 2] = u - prevU;
      aIntTexCoords[i * 2 + 1] = v - prevV;
    }
    else
    {
      aIntTexCoords[i * 2] = u;
      aIntTexCoords[i * 2 + 1] = v;
    }

    prevU = u;
    prevV = v;
    prevGridIdx = gridIdx;
  }
}

//-----------------------------------------------------------------------------
// _ctmRestoreTexCoords() - Calculate inverse derivatives of the texture
// coordinates.
//-----------------------------------------------------------------------------
static void _ctmRestoreTexCoords(_CTMcontext * self, CTMint * aIntTexCoords,
  CTMuint * aGridIndices)
{
 CTMuint i;
 CTMint u, v, idx, prevIdx, prevU, prevV;
 CTMfloat scale;

  // Texture coordinate scaling factor
  scale = self->mTexCoordPrecision;

  prevIdx = 0x7fffffff;
  prevU = prevV = 0;
  for(i = 0; i < self->mVertexCount; ++ i)
  {
    // Get grid box index
    idx = aGridIndices[i];

    // Calculate inverse delta (when feasible)
    if(idx == prevIdx)
    {
      u = aIntTexCoords[i * 2] + prevU;
      v = aIntTexCoords[i * 2 + 1] + prevV;
    }
    else
    {
      u = aIntTexCoords[i * 2];
      v = aIntTexCoords[i * 2 + 1];
    }

    // Convert to floating point
    self->mTexCoords[i * 2] = (CTMfloat) u * scale;
    self->mTexCoords[i * 2 + 1] = (CTMfloat) v * scale;

    prevU = u;
    prevV = v;
    prevIdx = idx;
  }
}

//-----------------------------------------------------------------------------
// _ctmCompressMesh_MG2() - Compress the mesh that is stored in the CTM
// context, and write it the the output stream in the CTM context.
//-----------------------------------------------------------------------------
int _ctmCompressMesh_MG2(_CTMcontext * self)
{
  _CTMgrid grid;
  _CTMsortvertex * sortVertices;
  CTMuint * indices, * gridIndices;
  CTMint * intVertices, * intTexCoords;
  CTMuint i;

#ifdef __DEBUG_
  printf("COMPRESSION METHOD: MG2\n");
#endif

  // Setup 3D space subdivision grid
  _ctmSetupGrid(self, &grid);

  // Write MG2-specific header information to the stream
  _ctmStreamWrite(self, (void *) "HEAD", 4);
  _ctmStreamWriteUINT(self, 1); // MG2 header format version
  _ctmStreamWriteFLOAT(self, self->mVertexPrecision);
  _ctmStreamWriteFLOAT(self, self->mTexCoordPrecision);
  _ctmStreamWriteFLOAT(self, grid.mMin[0]);
  _ctmStreamWriteFLOAT(self, grid.mMin[1]);
  _ctmStreamWriteFLOAT(self, grid.mMin[2]);
  _ctmStreamWriteFLOAT(self, grid.mMax[0]);
  _ctmStreamWriteFLOAT(self, grid.mMax[1]);
  _ctmStreamWriteFLOAT(self, grid.mMax[2]);
  _ctmStreamWriteUINT(self, grid.mDivision[0]);
  _ctmStreamWriteUINT(self, grid.mDivision[1]);
  _ctmStreamWriteUINT(self, grid.mDivision[2]);

  // Prepare (sort) vertices
  sortVertices = (_CTMsortvertex *) malloc(sizeof(_CTMsortvertex) * self->mVertexCount);
  if(!sortVertices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  _ctmSortVertices(self, sortVertices, &grid);

  // Convert vertices to integers and calculate vertex deltas (entropy-reduction)
  intVertices = (CTMint *) malloc(sizeof(CTMint) * 3 * self->mVertexCount);
  if(!intVertices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    free((void *) sortVertices);
    return CTM_FALSE;
  }
  _ctmMakeVertexDeltas(self, intVertices, sortVertices, &grid);

  // Write vertices
#ifdef __DEBUG_
  printf("Vertices: ");
#endif
  _ctmStreamWrite(self, (void *) "VERT", 4);
  if(!_ctmStreamWritePackedInts(self, intVertices, self->mVertexCount, 3))
  {
    free((void *) intVertices);
    free((void *) sortVertices);
    return CTM_FALSE;
  }

  // Free temporary data for the vertices
  free((void *) intVertices);

  // Prepare grid indices (deltas)
  gridIndices = (CTMuint *) malloc(sizeof(CTMuint) * self->mVertexCount);
  if(!gridIndices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    free((void *) sortVertices);
    return CTM_FALSE;
  }
  gridIndices[0] = sortVertices[0].mGridIndex;
  for(i = 1; i < self->mVertexCount; ++ i)
    gridIndices[i] = sortVertices[i].mGridIndex - sortVertices[i - 1].mGridIndex;
  
  // Write grid indices
#ifdef __DEBUG_
  printf("Grid indices: ");
#endif
  _ctmStreamWrite(self, (void *) "GIDX", 4);
  if(!_ctmStreamWritePackedInts(self, (CTMint *) gridIndices, self->mVertexCount, 1))
  {
    free((void *) gridIndices);
    free((void *) sortVertices);
    return CTM_FALSE;
  }

  // Free temporary grid indices
  free((void *) gridIndices);

  // Perpare (sort) indices
  indices = (CTMuint *) malloc(sizeof(CTMuint) * self->mTriangleCount * 3);
  if(!indices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    free((void *) sortVertices);
    return CTM_FALSE;
  }
  if(!_ctmReIndexIndices(self, sortVertices, indices))
  {
    free((void *) indices);
    free((void *) sortVertices);
    return CTM_FALSE;
  }
  _ctmReArrangeTriangles(self, indices);

  // Calculate index deltas (entropy-reduction)
  _ctmMakeIndexDeltas(self, indices);

  // Write triangle indices
#ifdef __DEBUG_
  printf("Indices: ");
#endif
  _ctmStreamWrite(self, (void *) "INDX", 4);
  if(!_ctmStreamWritePackedInts(self, (CTMint *) indices, self->mTriangleCount, 3))
  {
    free((void *) indices);
    free((void *) sortVertices);
    return CTM_FALSE;
  }

  // Free temporary data for the indices
  free((void *) indices);

  if(self->mTexCoords)
  {
    // Convert texture coordinates to integers and calculate deltas (entropy-reduction)
    intTexCoords = (CTMint *) malloc(sizeof(CTMint) * 2 * self->mVertexCount);
    if(!intTexCoords)
    {
      self->mError = CTM_OUT_OF_MEMORY;
      free((void *) sortVertices);
      return CTM_FALSE;
    }
    _ctmMakeTexCoordDeltas(self, intTexCoords, sortVertices);

    // Write texture coordinates
#ifdef __DEBUG_
    printf("Texture coordinates: ");
#endif
    _ctmStreamWrite(self, (void *) "TEXC", 4);
    if(!_ctmStreamWritePackedInts(self, intTexCoords, self->mVertexCount, 2))
    {
      free((void *) sortVertices);
      free((void *) intTexCoords);
      return CTM_FALSE;
    }

    // Free temporary texture coordinate data
    free((void *) intTexCoords);
  }

  // Write normals (TEMPORARY HACK: no entropy reduction yet)
  if(self->mNormals)
  {
#ifdef __DEBUG_
    printf("Normals: ");
#endif
    _ctmStreamWrite(self, (void *) "NORM", 4);
    if(!_ctmStreamWritePackedFloats(self, self->mNormals, self->mVertexCount, 3))
    {
      free((void *) sortVertices);
      return CTM_FALSE;
    }
  }

  // Free temporary data
  free((void *) sortVertices);

  return CTM_TRUE;
}

//-----------------------------------------------------------------------------
// _ctmUncompressMesh_MG2() - Uncmpress the mesh from the input stream in the
// CTM context, and store the resulting mesh in the CTM context.
//-----------------------------------------------------------------------------
int _ctmUncompressMesh_MG2(_CTMcontext * self)
{
  CTMuint * indices, * gridIndices, i;
  CTMint * intVertices, * intTexCoords;
  _CTMgrid grid;

  // Read MG2-specific header information from the stream
  if(_ctmStreamReadUINT(self) != FOURCC("HEAD"))
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  if(_ctmStreamReadUINT(self) != 1) // MG2 header format version
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  self->mVertexPrecision = _ctmStreamReadFLOAT(self);
  if(self->mVertexPrecision <= 0.0)
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  self->mTexCoordPrecision = _ctmStreamReadFLOAT(self);
  if(self->mTexCoordPrecision <= 0.0)
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  grid.mMin[0] = _ctmStreamReadFLOAT(self);
  grid.mMin[1] = _ctmStreamReadFLOAT(self);
  grid.mMin[2] = _ctmStreamReadFLOAT(self);
  grid.mMax[0] = _ctmStreamReadFLOAT(self);
  grid.mMax[1] = _ctmStreamReadFLOAT(self);
  grid.mMax[2] = _ctmStreamReadFLOAT(self);
  if((grid.mMax[0] < grid.mMin[0]) ||
     (grid.mMax[1] < grid.mMin[1]) ||
     (grid.mMax[2] < grid.mMin[2]))
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  grid.mDivision[0] = _ctmStreamReadUINT(self);
  grid.mDivision[1] = _ctmStreamReadUINT(self);
  grid.mDivision[2] = _ctmStreamReadUINT(self);
  if((grid.mDivision[0] < 1) || (grid.mDivision[1] < 1) || (grid.mDivision[2] < 1))
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }

  // Initialize 3D space subdivision grid
  for(i = 0; i < 3; ++ i)
    grid.mSize[i] = (grid.mMax[i] - grid.mMin[i]) / grid.mDivision[i];

  // Read vertices
  if(_ctmStreamReadUINT(self) != FOURCC("VERT"))
  {
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  intVertices = (CTMint *) malloc(sizeof(CTMint) * self->mVertexCount * 3);
  if(!intVertices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    return CTM_FALSE;
  }
  if(!_ctmStreamReadPackedInts(self, intVertices, self->mVertexCount, 3))
  {
    free((void *) intVertices);
    return CTM_FALSE;
  }

  // Read grid indices
  if(_ctmStreamReadUINT(self) != FOURCC("GIDX"))
  {
    free((void *) intVertices);
    self->mError = CTM_FORMAT_ERROR;
    return CTM_FALSE;
  }
  gridIndices = (CTMuint *) malloc(sizeof(CTMuint) * self->mVertexCount);
  if(!gridIndices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    free((void *) intVertices);
    return CTM_FALSE;
  }
  if(!_ctmStreamReadPackedInts(self, (CTMint *) gridIndices, self->mVertexCount, 1))
  {
    free((void *) gridIndices);
    free((void *) intVertices);
    return CTM_FALSE;
  }

  // Restore grid indices (deltas)
  for(i = 1; i < self->mVertexCount; ++ i)
    gridIndices[i] += gridIndices[i - 1];

  // Restore vertices
  _ctmRestoreVertices(self, intVertices, gridIndices, &grid);

  // Free temporary integer vertices
  free((void *) intVertices);

  // Read triangle indices
  indices = (CTMuint *) malloc(sizeof(CTMuint) * self->mTriangleCount * 3);
  if(!indices)
  {
    self->mError = CTM_OUT_OF_MEMORY;
    free(gridIndices);
    return CTM_FALSE;
  }
  if(_ctmStreamReadUINT(self) != FOURCC("INDX"))
  {
    self->mError = CTM_FORMAT_ERROR;
    free(indices);
    free(gridIndices);
    return CTM_FALSE;
  }
  if(!_ctmStreamReadPackedInts(self, (CTMint *) indices, self->mTriangleCount, 3))
  {
    free((void *) indices);
    free(gridIndices);
    return CTM_FALSE;
  }

  // Restore indices
  _ctmRestoreIndices(self, indices);
  for(i = 0; i < self->mTriangleCount * 3; ++ i)
    self->mIndices[i] = indices[i];

  // Free temporary indices
  free(indices);

  // Read texture coordintes
  if(self->mTexCoords)
  {
    intTexCoords = (CTMint *) malloc(sizeof(CTMint) * self->mVertexCount * 2);
    if(!indices)
    {
      self->mError = CTM_OUT_OF_MEMORY;
      free((void *) gridIndices);
      return CTM_FALSE;
    }
    if(_ctmStreamReadUINT(self) != FOURCC("TEXC"))
    {
      self->mError = CTM_FORMAT_ERROR;
      free((void *) intTexCoords);
      free((void *) gridIndices);
      return CTM_FALSE;
    }
    if(!_ctmStreamReadPackedInts(self, intTexCoords, self->mVertexCount, 2))
    {
      free((void *) intTexCoords);
      free((void *) gridIndices);
      return CTM_FALSE;
    }

    // Restore texture coordinates
    _ctmRestoreTexCoords(self, intTexCoords, gridIndices);

    // Free temporary texture coordinate data
    free((void *) intTexCoords);
  }

  // Read normals (TEMPORARY HACK: no entropy reduction yet)
  if(self->mNormals)
  {
    if(_ctmStreamReadUINT(self) != FOURCC("NORM"))
    {
      self->mError = CTM_FORMAT_ERROR;
      free((void *) gridIndices);
      return CTM_FALSE;
    }
    if(!_ctmStreamReadPackedFloats(self, self->mNormals, self->mVertexCount, 3))
    {
      free((void *) gridIndices);
      return CTM_FALSE;
    }
  }

  // Free temporary grid indices
  free((void *) gridIndices);

  return CTM_TRUE;
}
