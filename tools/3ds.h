#ifndef __3DS_H_
#define __3DS_H_

#include "mesh.h"

/// Import a 3DS file from a file.
void Import_3DS(const char * aFileName, Mesh &aMesh);

/// Export a 3DS file to a file.
void Export_3DS(const char * aFileName, Mesh &aMesh);

#endif // __3DS_H_
