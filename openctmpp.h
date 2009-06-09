//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        openctmpp.h
// Description: C++ wrapper for the OpenCTM API.
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

#ifndef __OPENCTMPP_H_
#define __OPENCTMPP_H_

// Just in case (if this file was included from outside openctm.h)...
#ifndef __OPENCTM_H_
#include "openctm.h"
#endif


/// OpenCTM importer class. This is a C++ wrapper class for an OpenCTM import
/// context. Usage example:
///
/// @code
///   // Create a new OpenCTM importer object
///   CTMimporter ctm;
///
///   // Load the OpenCTM file
///   ctm.Load("mymesh.ctm");
///
///   // Access the mesh data
///   vertCount = ctm.GetInteger(CTM_VERTEX_COUNT);
///   vertices = ctm.GetFloatArray(CTM_VERTICES);
///   triCount = ctm.GetInteger(CTM_TRIANGLE_COUNT);
///   indices = ctm.GetIntegerArray(CTM_INDICES);
///
///   // Deal with the mesh (e.g. transcode it to our internal representation)
///   // ...
/// @endcode

class CTMimporter {
  private:
    /// The OpenCTM context handle.
    CTMcontext mContext;

  public:
    /// Constructor
    CTMimporter()
    {
      mContext = ctmNewContext(CTM_IMPORT);
    }

    /// Destructor
    ~CTMimporter()
    {
      ctmFreeContext(mContext);
    }

    /// Wrapper for ctmGetError()
    CTMenum GetError()
    {
      return ctmGetError(mContext);
    }

    /// Wrapper for ctmGetInteger()
    CTMuint GetInteger(CTMenum aProperty)
    {
      return ctmGetInteger(mContext, aProperty);
    }

    /// Wrapper for ctmGetError()
    const CTMuint * GetIntegerArray(CTMenum aProperty)
    {
      return ctmGetIntegerArray(mContext, aProperty);
    }

    /// Wrapper for ctmGetFloatArray()
    const CTMfloat * GetFloatArray(CTMenum aProperty)
    {
      return ctmGetFloatArray(mContext, aProperty);
    }

    /// Wrapper for ctmGetNamedTexMap()
    CTMenum GetNamedTexMap(const char * aName)
    {
      return ctmGetNamedTexMap(mContext, aName);
    }

    /// Wrapper for ctmGetTexMapString()
    const char * GetTexMapString(CTMenum aTexMap, CTMenum aProperty)
    {
      return ctmGetTexMapString(mContext, aTexMap, aProperty);
    }

    /// Wrapper for ctmGetNamedAttribMap()
    CTMenum GetNamedAttribMap(const char * aName)
    {
      return ctmGetNamedAttribMap(mContext, aName);
    }

    /// Wrapper for ctmGetAttribMapString()
    const char * GetAttribMapString(CTMenum aAttribMap, CTMenum aProperty)
    {
      return ctmGetAttribMapString(mContext, aAttribMap, aProperty);
    }

    /// Wrapper for ctmGetString()
    const char * GetString(CTMenum aProperty)
    {
      return ctmGetString(mContext, aProperty);
    }

    /// Wrapper for ctmLoad()
    void Load(const char * aFileName)
    {
      ctmLoad(mContext, aFileName);
    }

    /// Wrapper for ctmLoadCustom()
    void LoadCustom(CTMreadfn aReadFn, void * aUserData)
    {
      ctmLoadCustom(mContext, aReadFn, aUserData);
    }

    // You can not copy nor assign from one CTMimporter object to another, since
    // the object contains hidden state. By declaring these dummy prototypes
    // without an implementation, you will at least get linker errors if you try
    // to copy or assign a CTMimporter object.
    CTMimporter(const CTMimporter& v);
    CTMimporter& operator=(const CTMimporter& v);
};


/// OpenCTM exporter class. This is a C++ wrapper class for an OpenCTM export
/// context. Usage example:
/// @code
/// void MySaveFile(CTMuint aVertCount, CTMuint aTriCount, CTMfloat * aVertices,
///   CTMuint * aIndices, const char * aFileName)
/// {
///   // Create a new OpenCTM exporter object
///   CTMexporter ctm;
///
///   // Define our mesh representation to OpenCTM (store references to it in
///   // the context)
///   ctm.DefineMesh(aVertices, aVertCount, aIndices, aTriCount, NULL);
///
///   // Save the OpenCTM file
///   ctm.Save(aFileName);
/// }
/// @endcode

class CTMexporter {
  private:
    /// The OpenCTM context handle.
    CTMcontext mContext;

  public:
    /// Constructor
    CTMexporter()
    {
      mContext = ctmNewContext(CTM_EXPORT);
    }

    /// Destructor
    ~CTMexporter()
    {
      ctmFreeContext(mContext);
    }

    /// Wrapper for ctmGetError()
    CTMenum GetError()
    {
      return ctmGetError(mContext);
    }

    /// Wrapper for ctmCompressionMethod()
    void CompressionMethod(CTMenum aMethod)
    {
      ctmCompressionMethod(mContext, aMethod);
    }

    /// Wrapper for ctmVertexPrecision()
    void VertexPrecision(CTMfloat aPrecision)
    {
      ctmVertexPrecision(mContext, aPrecision);
    }

    /// Wrapper for ctmVertexPrecisionRel()
    void VertexPrecisionRel(CTMfloat aRelPrecision)
    {
      ctmVertexPrecisionRel(mContext, aRelPrecision);
    }

    /// Wrapper for ctmNormalPrecision()
    void NormalPrecision(CTMfloat aPrecision)
    {
      ctmNormalPrecision(mContext, aPrecision);
    }

    /// Wrapper for ctmTexCoordPrecision()
    void TexCoordPrecision(CTMenum aTexMap, CTMfloat aPrecision)
    {
      ctmTexCoordPrecision(mContext, aTexMap, aPrecision);
    }

    /// Wrapper for ctmAttribPrecision()
    void AttribPrecision(CTMenum aAttribMap, CTMfloat aPrecision)
    {
      ctmAttribPrecision(mContext, aAttribMap, aPrecision);
    }

    /// Wrapper for ctmFileComment()
    void FileComment(const char * aFileComment)
    {
      ctmFileComment(mContext, aFileComment);
    }

    /// Wrapper for ctmDefineMesh()
    void DefineMesh(const CTMfloat * aVertices, CTMuint aVertexCount, 
      const CTMuint * aIndices, CTMuint aTriangleCount,
      const CTMfloat * aNormals)
    {
      ctmDefineMesh(mContext, aVertices, aVertexCount, aIndices, aTriangleCount,
                    aNormals);
    }

    /// Wrapper for ctmAddTexMap()
    CTMenum AddTexMap(const CTMfloat * aTexCoords, const char * aName,
      const char * aFileName)
    {
      return ctmAddTexMap(mContext, aTexCoords, aName, aFileName);
    }

    /// Wrapper for ctmAddAttribMap()
    CTMenum AddAttribMap(const CTMfloat * aAttribValues, const char * aName)
    {
      return ctmAddAttribMap(mContext, aAttribValues, aName);
    }

    /// Wrapper for ctmSave()
    void Save(const char * aFileName)
    {
      ctmSave(mContext, aFileName);
    }

    /// Wrapper for ctmSaveCustom()
    void SaveCustom(CTMwritefn aWriteFn, void * aUserData)
    {
      ctmSaveCustom(mContext, aWriteFn, aUserData);
    }

    // You can not copy nor assign from one CTMexporter object to another, since
    // the object contains hidden state. By declaring these dummy prototypes
    // without an implementation, you will at least get linker errors if you try
    // to copy or assign a CTMexporter object.
    CTMexporter(const CTMexporter& v);
    CTMexporter& operator=(const CTMexporter& v);
};

#endif // __OPENCTMPP_H_
