//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        image.h
// Description: Interface for the Image class.
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

#ifndef __IMAGE_H_
#define __IMAGE_H_

#include <vector>

class Image {
  public:
    /// Constructor
    Image()
    {
      mWidth = mHeight = 0;
      mComponents = 4;
    }

    /// Load an image from a file
    void LoadFromFile(const char * aFileName);

    /// Image width (in pixels)
    int mWidth;

    /// Image height (in pixels)
    int mHeight;

    /// Number of components (1,3 or 4)
    int mComponents;

    /// Pixel data
    std::vector<unsigned char> mData;
};


#endif // __IMAGE_H_
