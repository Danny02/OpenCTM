#ifndef __CTM_H_
#define __CTM_H_

#include "mesh.h"
#include "convoptions.h"

/// Import an OpenCTM file from a file.
void Import_CTM(const char * aFileName, Mesh &aMesh);

/// Export an OpenCTM file to a file.
void Export_CTM(const char * aFileName, Mesh &aMesh, Options &aOptions);

#endif // __CTM_H_
