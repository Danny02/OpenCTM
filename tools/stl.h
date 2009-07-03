#ifndef __STL_H_
#define __STL_H_

#include "mesh.h"

/// Import an STL file from a file.
void Import_STL(const char * aFileName, Mesh &aMesh);

/// Export an STL file to a file.
void Export_STL(const char * aFileName, Mesh &aMesh);

#endif // __STL_H_
