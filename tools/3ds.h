#ifndef __3DS_H_
#define __3DS_H_

#include <istream>
#include <ostream>
#include "mesh.h"

/// Import a 3DS file from a stream.
void Import_3DS(std::istream &aStream, Mesh &aMesh);

/// Export a 3DS file to a stream.
void Export_3DS(std::ostream &aStream, Mesh &aMesh);

#endif // __3DS_H_
