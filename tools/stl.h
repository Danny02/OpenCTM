#ifndef __STL_H_
#define __STL_H_

#include <istream>
#include <ostream>
#include "mesh.h"

/// Import an STL file from a stream.
void STL_Import(std::istream &aStream, Mesh &aMesh);

/// Export an STL file to a stream.
void STL_Export(std::ostream &aStream, Mesh &aMesh);

#endif // __STL_H_
