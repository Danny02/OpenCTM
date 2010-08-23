//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        array.c
// Description: Typed array access functions.
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


//-----------------------------------------------------------------------------
// _ctmGetArrayi() - Get an integer value from a typed array.
//-----------------------------------------------------------------------------
CTMuint _ctmGetArrayi(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent)
{
  if((aComponent < aArray->mSize) && aArray->mData)
  {
    void * elementPtr = (void *) &((CTMbyte *)aArray->mData)[aElement * aArray->mStride];
    switch(aArray->mType)
    {
      case CTM_BYTE:
        return (CTMuint) ((CTMbyte *)elementPtr)[aComponent];
      case CTM_UBYTE:
        return (CTMuint) ((CTMubyte *)elementPtr)[aComponent];
      case CTM_SHORT:
        return (CTMuint) ((CTMshort *)elementPtr)[aComponent];
      case CTM_USHORT:
        return (CTMuint) ((CTMushort *)elementPtr)[aComponent];
      case CTM_INT:
        return (CTMuint) ((CTMint *)elementPtr)[aComponent];
      case CTM_UINT:
        return ((CTMuint *)elementPtr)[aComponent];
      case CTM_FLOAT:
        return (CTMuint) ((CTMfloat *)elementPtr)[aComponent];
      case CTM_DOUBLE:
        return (CTMuint) ((CTMdouble *)elementPtr)[aComponent];
      default:
        break;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
// _ctmGetArrayf() - Get a floating point value from a typed array.
//-----------------------------------------------------------------------------
CTMfloat _ctmGetArrayf(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent)
{
  if((aComponent < aArray->mSize) && aArray->mData)
  {
    void * elementPtr = (void *) &((CTMbyte *)aArray->mData)[aElement * aArray->mStride];
    switch(aArray->mType)
    {
      case CTM_BYTE:
        return (1.0f/127.0f) * (CTMfloat) ((CTMbyte *)elementPtr)[aComponent];
      case CTM_UBYTE:
        return (1.0f/255.0f) * (CTMfloat) ((CTMubyte *)elementPtr)[aComponent];
      case CTM_SHORT:
        return (CTMfloat) ((CTMshort *)elementPtr)[aComponent];
      case CTM_USHORT:
        return (CTMfloat) ((CTMushort *)elementPtr)[aComponent];
      case CTM_INT:
        return (CTMfloat) ((CTMint *)elementPtr)[aComponent];
      case CTM_UINT:
        return (CTMfloat) ((CTMuint *)elementPtr)[aComponent];
      case CTM_FLOAT:
        return ((CTMfloat *)elementPtr)[aComponent];
      case CTM_DOUBLE:
        return (CTMfloat) ((CTMdouble *)elementPtr)[aComponent];
      default:
        break;
    }
  }
  return 0.0f;
}

//-----------------------------------------------------------------------------
// _ctmSetArrayi() - Set an integer value in a typed array.
//-----------------------------------------------------------------------------
void _ctmSetArrayi(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent,
  CTMuint aValue)
{
  if((aComponent < aArray->mSize) && aArray->mData)
  {
    void * elementPtr = (void *) &((CTMbyte *)aArray->mData)[aElement * aArray->mStride];
    switch(aArray->mType)
    {
      case CTM_BYTE:
        ((CTMbyte *)elementPtr)[aComponent] = (CTMbyte) aValue;
        break;
      case CTM_UBYTE:
        ((CTMubyte *)elementPtr)[aComponent] = (CTMubyte) aValue;
        break;
      case CTM_SHORT:
        ((CTMshort *)elementPtr)[aComponent] = (CTMshort) aValue;
        break;
      case CTM_USHORT:
        ((CTMushort *)elementPtr)[aComponent] = (CTMushort) aValue;
        break;
      case CTM_INT:
        ((CTMint *)elementPtr)[aComponent] = (CTMint) aValue;
        break;
      case CTM_UINT:
        ((CTMuint *)elementPtr)[aComponent] = aValue;
        break;
      case CTM_FLOAT:
        ((CTMfloat *)elementPtr)[aComponent] = (CTMfloat) aValue;
        break;
      case CTM_DOUBLE:
        ((CTMdouble *)elementPtr)[aComponent] = (CTMdouble) aValue;
        break;
      default:
        break;
    }
  }
}

//-----------------------------------------------------------------------------
// _ctmSetArrayf() - Set a floating point value in a typed array.
//-----------------------------------------------------------------------------
void _ctmSetArrayf(_CTMarray * aArray, CTMuint aElement, CTMuint aComponent,
  CTMfloat aValue)
{
  if((aComponent < aArray->mSize) && aArray->mData)
  {
    void * elementPtr = (void *) &((CTMbyte *)aArray->mData)[aElement * aArray->mStride];
    switch(aArray->mType)
    {
      case CTM_BYTE:
        ((CTMbyte *)elementPtr)[aComponent] = (CTMbyte) (127.0f * aValue);
        break;
      case CTM_UBYTE:
        ((CTMubyte *)elementPtr)[aComponent] = (CTMubyte) (255.0f * aValue);
        break;
      case CTM_SHORT:
        ((CTMshort *)elementPtr)[aComponent] = (CTMshort) aValue;
        break;
      case CTM_USHORT:
        ((CTMushort *)elementPtr)[aComponent] = (CTMushort) aValue;
        break;
      case CTM_INT:
        ((CTMint *)elementPtr)[aComponent] = (CTMint) aValue;
        break;
      case CTM_UINT:
        ((CTMuint *)elementPtr)[aComponent] = (CTMuint) aValue;
        break;
      case CTM_FLOAT:
        ((CTMfloat *)elementPtr)[aComponent] = aValue;
        break;
      case CTM_DOUBLE:
        ((CTMdouble *)elementPtr)[aComponent] = (CTMdouble) aValue;
        break;
      default:
        break;
    }
  }
}
