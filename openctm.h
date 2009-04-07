//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        openctm.h
// Description: OpenCTM API definition.
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

#ifndef __OPENCTM_H_
#define __OPENCTM_H_

/*! @mainpage OpenCTM Documentation
 *
 * @section intro_sec Introduction
 *
 * OpenCTM is an open file format for storing compressed triangle meshes.
 * In order to easily read and write OpenCTM files (usually suffixed .ctm) an
 * API (Application Program Interface) is provided that can easily be used from
 * most modern programming languages.
 *
 * The OpenCTM functionality itself is written in highly portable standard C
 * (C99).
 *
 * @section usage_sec Usage
 *
 * For information about how to use the OpenCTM API, see openctm.h.
 *
 * @section example_sec Example usage
 *
 * @subsection example_load_sec Loading a CTM file
 *
 * Here is a simple example of loading a CTM file:
 *
 * @code
 *   CTMcontext context;
 *   CTMuint    vertCount, triCount, * indices;
 *   CTMfloat   * vertices;
 *
 *   // Create a new context
 *   context = ctmNewContext();
 *
 *   // Load the OpenCTM file
 *   ctmLoad(context, "mymesh.ctm");
 *
 *   // Transfer the mesh to our own mesh representation
 *   vertCount = ctmGetInteger(context, CTM_VERTEX_COUNT);
 *   triCount = ctmGetInteger(context, CTM_TRIANGLE_COUNT);
 *   vertices = (CTMfloat *) malloc(3 * sizeof(CTMfloat) * vertCount);
 *   indices = (CTMuint *) malloc(3 * sizeof(CTMuint) * triCount);
 *   ctmGetMesh(context, vertices, vertCount, indices, triCount, NULL, NULL);
 *
 *   // Free the context
 *   ctmFreeContext(context);
 *
 *   // Deal with our mesh
 *   // ...
 *
 *   //Free our mesh
 *   free(indices);
 *   free(vertices);
 * @endcode
 *
 * @subsection example_create_sec Creating a CTM file
 *
 * Here is a simple example of creating a CTM file:
 *
 * @code
 *   CTMcontext context;
 *   CTMuint    vertCount, triCount, * indices;
 *   CTMfloat   * vertices;
 *
 *   // Create our mesh in memory
 *   vertCount = 100;
 *   triCount = 120;
 *   vertices = (CTMfloat *) malloc(3 * sizeof(CTMfloat) * vertCount);
 *   indices = (CTMuint *) malloc(3 * sizeof(CTMuint) * triCount);
 *   // ...
 *
 *   // Create a new context
 *   context = ctmNewContext();
 *
 *   // Transfer the mesh from our own mesh representation to the OpenCTM
 *   // context
 *   ctmDefineMesh(context, vertices, vertCount, indices, triCount, NULL, NULL);
 *
 *   //Free our mesh
 *   free(indices);
 *   free(vertices);
 *
 *   // Save the OpenCTM file
 *   ctmSave(context, "mymesh.ctm");
 *
 *   // Free the context
 *   ctmFreeContext(context);
 * @endcode
 *
 * @section format_sec File format
 *
 * The file format is quite complex, since OpenCTM uses several steps of data
 * processing in order to reduce as much redundant information as possible.
 *
 * Some of the techniques that are used are:
 * @li Data reordering.
 * @li Differentiation ("delta to neighbours").
 * @li Data interleaving.
 * @li Entropy coding (LZMA).
 *
 * Because of the complexity of these data operations, it is left to the user
 * of OpenCTM to read the commented source code as a substitute for a plain
 * text specification of the file format.
 */

#ifdef __cplusplus
extern "C" {
#endif

/// Boolean TRUE.
#define CTM_TRUE 1

/// Boolean FALSE.
#define CTM_FALSE 0

/// OpenCTM file format version (1.0).
#define CTM_FORMAT_VERSION 0x0100

/// Single precision floating point type.
typedef float CTMfloat;

/// Signed integer (at least 32 bits wide).
typedef int CTMint;

/// Unsigned integer (at least 32 bits wide).
typedef unsigned int CTMuint;

/// OpenCTM context handle.
typedef void * CTMcontext;

/// Error codes. All error codes are positive values, except for CTM_NO_ERROR,
/// which is zero.
typedef enum {
  CTM_NO_ERROR         = 0, ///< No error (everything is OK).
  CTM_INVALID_CONTEXT  = 1, ///< The OpenCTM context was invalid (e.g. NULL).
  CTM_INVALID_ARGUMENT = 2, ///< A function argument was invalid.
  CTM_INVALID_MESH     = 3, ///< The mesh was invalid (e.g. no vertices).
  CTM_OUT_OF_MEMORY    = 4, ///< Not enough memory to proceed.
  CTM_FILE_ERROR       = 5, ///< File I/O error.
  CTM_FORMAT_ERROR     = 6, ///< File format error (e.g. unrecognized format).
  CTM_LZMA_ERROR       = 7  ///< An error occured within the LZMA library.
} CTMerror;

/// Arguments for the ctmGetInteger() and ctmGetString() functions.
/// @note It is an error to query a string value with the ctmGetInteger()
///       function, or to query an integer value with the ctmGetString()
///       function.
typedef enum {
  CTM_VERTEX_COUNT = 1,   ///< Number of vertices in the mesh (integer).
  CTM_TRIANGLE_COUNT = 2, ///< Number of triangles in the mesh (integer).
  CTM_HAS_TEX_COORDS = 3, ///< CTM_TRUE if the mesh has texture coordinates (integer).
  CTM_HAS_NORMALS = 4,    ///< CTM_TRUE if the mesh has normals (integer).
  CTM_FILE_COMMENT = 5    ///< File comment (string).
} CTMproperty;

/// Supported compression methods.
typedef enum {
  CTM_METHOD_RAW = 1,   ///< Just store the raw data.
  CTM_METHOD_MG1 = 2,   ///< Lossless compression.
  CTM_METHOD_MG2 = 3    ///< Fairly advanced, slightly "lossy" compression.
} CTMmethod;

/// Stream read() function pointer.
/// @param[in] aBuf Pointer to the memory buffer to which data should be read.
/// @param[in] aCount The number of bytes to read.
/// @param[in] aUserData The custom user data that was passed to the
///             ctmLoadCustom() function.
/// @return The number of bytes actually read (if this is less than aCount, it
///         indicates that an error occured or the end of file was reached).
typedef CTMuint (* CTMreadfn)(void * aBuf, CTMuint aCount, void * aUserData);

/// Stream write() function pointer.
/// @param[in] aBuf Pointer to the memory buffer from which data should be written.
/// @param[in] aCount The number of bytes to write.
/// @param[in] aUserData The custom user data that was passed to the
///             ctmSaveCustom() function.
/// @return The number of bytes actually written (if this is less than aCount, it
///         indicates that an error occured).
typedef CTMuint (* CTMwritefn)(const void * aBuf, CTMuint aCount, void * aUserData);

/// Create a new OpenCTM context. The context is used for all subsequent
/// OpenCTM function calls. Several contexts can coexist at the same time.
/// @return An OpenCTM context handle (or NULL if no context could be created).
CTMcontext ctmNewContext(void);

/// Free an OpenCTM context.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @see ctmNewContext()
void ctmFreeContext(CTMcontext aContext);

/// Returns the latest error. Calling this function will return the last
/// produced error code, or CTM_NO_ERROR (zero) if no error has occured since
/// the last call to ctmError(). When this function is called, the internal
/// error varibale will be reset to CTM_NO_ERROR.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @return An OpenCTM error code.
/// @see CTMerror
CTMerror ctmError(CTMcontext aContext);

/// Get information about an OpenCTM context.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aProperty Which property to return.
/// @return An integer value, representing the OpenCTM context property given
///         by \c aProperty.
/// @see CTMproperty
CTMuint ctmGetInteger(CTMcontext aContext, CTMproperty aProperty);

/// Get information about an OpenCTM context.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aProperty Which property to return.
/// @return A string value, representing the OpenCTM context property given
///         by \c aProperty.
/// @note The string is only valid as long as the OpenCTM context is valid, or
///       until the corresponding string changes within the OpenCTM contxt
///       (e.g. calling ctmFileComment() invalidates the CTM_FILE_COMMENT
///       string). Trying to access an invalid string will result in undefined
///       behaviour. Therefor it is recommended that the string is copied to
///       a new variable if it is to be used other than directly after the call
///       to ctmGetString().
/// @see CTMproperty
const char * ctmGetString(CTMcontext aContext, CTMproperty aProperty);

/// Set which compression method to use for the given OpenCTM context.
/// The selected compression method will be used when calling the ctmSave()
/// function.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aMethod Which compression method to use (the default method is
///            CTM_METHOD_MG1).
/// @see CTMmethod
void ctmCompressionMethod(CTMcontext aContext, CTMmethod aMethod);

/// Set the vertex coordinate precision (only used by the MG2 compression
/// method).
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aPrecision Fixed point precision. For instance, if this value is
///             0.001, all coordinates will be rounded to three decimals.
void ctmVertexPrecision(CTMcontext aContext, CTMfloat aPrecision);

/// Set the file comment for the given OpenCTM context.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aFileComment The file comment (zero terminated UTF-8 string).
void ctmFileComment(CTMcontext aContext, const char * aFileComment);

/// Define a triangle mesh.
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aVertices An array of vertices (three consecutive floats make
///            one vertex).
/// @param[in] aVertexCount The number of vertices in \c aVertices (and
///            optionally \c aTexCoords).
/// @param[in] aIndices An array of vertex indices (three consecutive integers
///            make one triangle).
/// @param[in] aTriangleCount The number of triangles in \c aIndices (there
///            must be exactly 3 x \c aTriangleCount indices in \c aIndices).
/// @param[in] aTexCoords An array of texture coordinates (or NULL if there are
///            no texture coordinates). Each texture coordinate is made up by
///            two consecutive floats, and there must be \c aVertexCount
///            texture coordinates.
/// @param[in] aNormals An array of per-vertex normals (or NULL if there are
///            no normals). Each nromal is made up by three consecutive floats,
///            and there must be \c aVertexCount normals. All normals must have
///            unit length.
/// @see ctmSave(), ctmSaveCustom().
void ctmDefineMesh(CTMcontext aContext, const CTMfloat * aVertices,
                   CTMuint aVertexCount, const CTMuint * aIndices,
                   CTMuint aTriangleCount, const CTMfloat * aTexCoords,
                   const CTMfloat * aNormals);

/// Retrieve the triangle mesh from the OpenCTM context. The arrays must have
/// been allocated by the caller, and should have enough capacity to hold the
/// entire mesh (use ctmGetInteger() to determine the size of the mesh).
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aVertices An array that will receive the vertices (three
///            consecutive floats make one vertex).
/// @param[in] aVertexCount The number of vertices that \c aVertices (and
///            optionally \c aTexCoords) can hold.
/// @param[in] aIndices An array that will receive the vertex indices (three
///            consecutive integers make one triangle).
/// @param[in] aTriangleCount The number of triangles that \c aIndices can hold
///            (each triangle is made up of three indices).
/// @param[in] aTexCoords An array that will receive the texture coordinates
///            (or NULL if texture coordinates are not requested). Each texture
///            coordinate is made up by two consecutive floats, and there must
///            be \c aVertexCount texture coordinates. If the mesh does not
///            have any texture coordinates, and \c aTexCoords is non-NULL, the
///            array is zero-filled.
/// @param[in] aNormals An array that will receive the normals (or NULL if
///            normals are not requested). Each nromal is made up by three
///            consecutive floats, and there must be \c aVertexCount normals.
///            If the mesh does not have any normals, and \c aNormals is
///            non-NULL, the array is zero-filled..
/// @note The mesh should have been loaded by calling ctmLoad() or
///        ctmLoadCustom() prior to calling this function.
/// @see ctmGetInteger(), ctmLoad(), ctmLoadCustom().
void ctmGetMesh(CTMcontext aContext, CTMfloat * aVertices,
                CTMuint aVertexCount, CTMuint * aIndices,
                CTMuint aTriangleCount, CTMfloat * aTexCoords,
                CTMfloat * aNormals);

/// Load an OpenCTM format file. The mesh can be retrieved using ctmGetMesh().
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aFileName The name of the file to be loaded.
void ctmLoad(CTMcontext aContext, const char * aFileName);

/// Load an OpenCTM format file using a custom stream read function. The mesh
/// can be retrieved using ctmGetMesh().
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aReadFn Pointer to a custom stream read function.
/// @param[in] aUserData Custom user data, which can be a C language FILE
///             handle, C++ istream object, or a custom object pointer
///             of any type. The user data pointer will be passed to the
///             custom stream read function.
/// @see CTMreadfn.
void ctmLoadCustom(CTMcontext aContext, CTMreadfn aReadFn, void * aUserData);

/// Save an OpenCTM format file. The mesh must have been defined by
/// ctmDefineMesh() (or possibly by ctmLoad()).
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aFileName The name of the file to be saved.
void ctmSave(CTMcontext aContext, const char * aFileName);

/// Save an OpenCTM format file using a custom stream write function. The mesh
/// must have been defined by ctmDefineMesh() (or possibly by ctmLoad()).
/// @param[in] aContext An OpenCTM context that has been created by
///            ctmNewContext().
/// @param[in] aWriteFn Pointer to a custom stream write function.
/// @param[in] aUserData Custom user data, which can be a C language FILE
///             handle, C++ ostream object, or a custom object pointer
///             of any type. The user data pointer will be passed to the
///             custom stream write function.
/// @see CTMwritefn.
void ctmSaveCustom(CTMcontext aContext, CTMwritefn aWriteFn, void * aUserData);

#ifdef __cplusplus
}
#endif

#endif // __OPENCTM_H_
