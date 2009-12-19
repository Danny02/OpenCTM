//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        image.cpp
// Description: Implementation of the Image class.
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

#include <stdexcept>
#include "image.h"
#include "common.h"

using namespace std;

/// Load an image from a file
void Image::LoadFromFile(const char * aFileName)
{
  string fileExt = UpperCase(ExtractFileExt(string(aFileName)));
  if((fileExt == string(".JPG")) || (fileExt == string(".JPEG")))
    LoadJPEG(aFileName);
  else if(fileExt == string(".PNG")))
    LoadPNG(aFileName);
  else
    throw runtime_error("Unknown input file extension.");
}

/// Load image from a JPEG file.
void Image::LoadJPEG(const char * aFileName)
{
  throw runtime_error("JPEG import is not yet implemented.");
}

/// Load image from a PNG file.
void Image::LoadPNG(const char * aFileName)
{
  throw runtime_error("PNG import is not yet implemented.");
}
