#ifndef __PLY_H_
#define __PLY_H_

#include <istream>
#include <ostream>
#include "mesh.h"

/// Import a PLY file to a stream.
void PLY_Import(std::istream &aStream, Mesh &aMesh);

/// Export a PLY file to a stream.
void PLY_Export(std::ostream &aStream, Mesh &aMesh);

#endif // __PLY_H_
