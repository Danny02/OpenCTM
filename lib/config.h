//-----------------------------------------------------------------------------
// Product:     OpenCTM
// File:        config.h
// Description: Build time configuration of the library. Edit this to make the
//              library fit your needs.
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

#ifndef __OPENCTM_CONFIG_H_
#define __OPENCTM_CONFIG_H_


//-----------------------------------------------------------------------------
// Enable/disable parts of the library.
//-----------------------------------------------------------------------------

// Enable support for the RAW method.
#define _CTM_SUPPORT_RAW

// Enable support for the MG1 method.
#define _CTM_SUPPORT_MG1

// Enable support for the MG2 method.
#define _CTM_SUPPORT_MG2

// Enable support for saving OpenCTM files.
#define _CTM_SUPPORT_SAVE

// Enable multi threaded operation.
#define _CTM_SUPPORT_MT

// Enable support for loading file format v5 (OpenCTM 1.x files).
#define _CTM_SUPPORT_V5_FILES

//-----------------------------------------------------------------------------
// Default compression parameters.
//-----------------------------------------------------------------------------

// Default LZMA compression level (0 - 9)·
#define _CTM_DEFAULT_LZMA_LEVEL 1

// Default vertex coordinate precision.
#define _CTM_DEFAULT_VERTEX_PRECISION (1.0f / 1024.0f)

// Default normal precision.
#define _CTM_DEFAULT_NORMAL_PRECISION (1.0f / 256.0f)

// Default UV coordinate precision
#define _CTM_DEFAULT_UV_PRECISION (1.0f / 4096.0f)

// Default vertex attribute precision
#define _CTM_DEFAULT_ATTRIB_PRECISION (1.0f / 256.0f)


#endif // __OPENCTM_CONFIG_H_
