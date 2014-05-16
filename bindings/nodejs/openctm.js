//------------------------------------------------------------------------------
// Product:     OpenCTM
// File:        openctm.js
// Description: Node.js API bindings
//------------------------------------------------------------------------------
// Copyright (c) 2013 DFKI
// Author: Dmitri Rubinstein
//
// Based on Python API bindings, openctm.py by Marcus Geelnard
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
//------------------------------------------------------------------------------

var ref = require('ref');
var ffi = require('ffi');

// Types
var CTMfloat = ref.types.float;
var CTMint = ref.types.int32;
var CTMuint = ref.types.uint32;
var CTMcontext = ref.refType(ref.types.void);
var CTMenum = ref.types.uint32;
var CTMreadfn = ref.refType(ref.types.void);
var CTMwritefn = ref.refType(ref.types.void);

exports.CTMfloat = CTMfloat;
exports.CTMint = CTMint;
exports.CTMuint = CTMuint;
exports.CTMcontext = CTMcontext;
exports.CTMenum = CTMenum;

// Constants
exports.CTM_API_VERSION = 0x00000100;
exports.CTM_TRUE = 1;
exports.CTM_FALSE = 0;

// CTMenum
exports.CTM_NONE = 0x0000;
exports.CTM_INVALID_CONTEXT = 0x0001;
exports.CTM_INVALID_ARGUMENT = 0x0002;
exports.CTM_INVALID_OPERATION = 0x0003;
exports.CTM_INVALID_MESH = 0x0004;
exports.CTM_OUT_OF_MEMORY = 0x0005;
exports.CTM_FILE_ERROR = 0x0006;
exports.CTM_BAD_FORMAT = 0x0007;
exports.CTM_LZMA_ERROR = 0x0008;
exports.CTM_INTERNAL_ERROR = 0x0009;
exports.CTM_UNSUPPORTED_FORMAT_VERSION = 0x000A;
exports.CTM_IMPORT = 0x0101;
exports.CTM_EXPORT = 0x0102;
exports.CTM_METHOD_RAW = 0x0201;
exports.CTM_METHOD_MG1 = 0x0202;
exports.CTM_METHOD_MG2 = 0x0203;
exports.CTM_VERTEX_COUNT = 0x0301;
exports.CTM_TRIANGLE_COUNT = 0x0302;
exports.CTM_HAS_NORMALS = 0x0303;
exports.CTM_UV_MAP_COUNT = 0x0304;
exports.CTM_ATTRIB_MAP_COUNT = 0x0305;
exports.CTM_VERTEX_PRECISION = 0x0306;
exports.CTM_NORMAL_PRECISION = 0x0307;
exports.CTM_COMPRESSION_METHOD = 0x0308;
exports.CTM_FILE_COMMENT = 0x0309;
exports.CTM_NAME = 0x0501;
exports.CTM_FILE_NAME = 0x0502;
exports.CTM_PRECISION = 0x0503;
exports.CTM_INDICES = 0x0601;
exports.CTM_VERTICES = 0x0602;
exports.CTM_NORMALS = 0x0603;
exports.CTM_UV_MAP_1 = 0x0700;
exports.CTM_UV_MAP_2 = 0x0701;
exports.CTM_UV_MAP_3 = 0x0702;
exports.CTM_UV_MAP_4 = 0x0703;
exports.CTM_UV_MAP_5 = 0x0704;
exports.CTM_UV_MAP_6 = 0x0705;
exports.CTM_UV_MAP_7 = 0x0706;
exports.CTM_UV_MAP_8 = 0x0707;
exports.CTM_ATTRIB_MAP_1 = 0x0800;
exports.CTM_ATTRIB_MAP_2 = 0x0801;
exports.CTM_ATTRIB_MAP_3 = 0x0802;
exports.CTM_ATTRIB_MAP_4 = 0x0803;
exports.CTM_ATTRIB_MAP_5 = 0x0804;
exports.CTM_ATTRIB_MAP_6 = 0x0805;
exports.CTM_ATTRIB_MAP_7 = 0x0806;
exports.CTM_ATTRIB_MAP_8 = 0x0807;

// Functions

switch (process.platform) {
	case 'win32': libname = 'openctm'; break;
	default: libname = 'libopenctm'; break;
}
var openctm = ffi.Library(libname, {
    'ctmNewContext' : [CTMcontext, [CTMenum]],
    'ctmFreeContext' : ['void', [CTMcontext]],
    'ctmGetError' : [CTMenum, [CTMcontext]],
    'ctmErrorString' : [ref.types.CString, [CTMenum]],
    'ctmGetInteger' : [CTMint, [CTMcontext, CTMenum]],
    'ctmGetFloat' : [CTMfloat, [CTMcontext, CTMenum]],
    'ctmGetIntegerArray' : [ref.refType(CTMuint), [CTMcontext, CTMenum]],
    'ctmGetFloatArray' : [ref.refType(CTMfloat), [CTMcontext, CTMenum]],
    'ctmGetNamedUVMap' : [CTMenum, [CTMcontext, ref.types.CString]],
    'ctmGetUVMapString' : [ref.types.CString, [CTMcontext, CTMenum, CTMenum]],
    'ctmGetUVMapFloat' : [CTMfloat, [CTMcontext, CTMenum, CTMenum]],
    'ctmGetNamedAttribMap' : [CTMenum, [CTMcontext, ref.types.CString]],
    'ctmGetAttribMapString' : [ref.types.CString, [CTMcontext, CTMenum, CTMenum]],
    'ctmGetAttribMapFloat' : [CTMfloat, [CTMcontext, CTMenum, CTMenum]],
    'ctmGetString' : [ref.types.CString, [CTMcontext, CTMenum]],
    'ctmCompressionMethod' : ['void', [CTMcontext, CTMenum]],
    'ctmCompressionLevel' : ['void', [CTMcontext, CTMuint]],
    'ctmVertexPrecision' : ['void', [CTMcontext, CTMfloat]],
    'ctmVertexPrecisionRel' : ['void', [CTMcontext, CTMfloat]],
    'ctmNormalPrecision' : ['void', [CTMcontext, CTMfloat]],
    'ctmUVCoordPrecision' : ['void', [CTMcontext, CTMenum, CTMfloat]],
    'ctmAttribPrecision' : ['void', [CTMcontext, CTMenum, CTMfloat]],
    'ctmFileComment' : ['void', [CTMcontext, ref.types.CString]],
    'ctmDefineMesh' : ['void', [CTMcontext, ref.refType(CTMfloat), CTMuint, ref.refType(CTMuint), CTMuint, ref.refType(CTMfloat)]],
    'ctmAddUVMap' : [CTMenum, [CTMcontext, ref.refType(CTMfloat), ref.types.CString, ref.types.CString]],
    'ctmAddAttribMap' : [CTMenum, [CTMcontext, ref.refType(CTMfloat), ref.types.CString]],
    'ctmLoad' : ['void', [CTMcontext, ref.types.CString]],
    'ctmLoadCustom' : ['void', [CTMcontext, CTMreadfn, 'void *']],
    'ctmSave' : ['void', [CTMcontext, ref.types.CString]],
    'ctmSaveCustom' : ['void', [CTMcontext, CTMwritefn, 'void *']],
    // extension
    'ctmSaveToBuffer' : ['void *', [CTMcontext, ref.refType(ref.types.size_t)]],
    'ctmFreeBuffer' : ['void', ['void *']]
});

for (name in openctm) {
    if (openctm.hasOwnProperty(name)) {
        exports[name] = openctm[name];
    }
}
