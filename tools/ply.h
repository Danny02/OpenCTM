#ifndef __PLY_H_
#define __PLY_H_

#include "mesh.h"

/// Import a PLY file from a file.
void Import_PLY(const char * aFileName, Mesh &aMesh);

/// Export a PLY file to a file.
void Export_PLY(const char * aFileName, Mesh &aMesh);

#endif // __PLY_H_
