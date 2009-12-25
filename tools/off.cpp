//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        off.cpp
// Description: Implementation of the OFF file format importer/exporter.
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

//-----------------------------------------------------------------------------
// The "Object File Format" (OFF) is used by the Princeton Shape Benchmark data
// set (http://shape.cs.princeton.edu/benchmark). The file format specification
// can be found here:
// http://shape.cs.princeton.edu/benchmark/documentation/off_format.html
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include "off.h"

using namespace std;


// Read the next line in a file (skip comments and empty lines)
static void ReadNextLine(ifstream &aStream, string &aResult)
{
  while(true)
  {
    string line;
    getline(aStream, line);
    size_t l = line.size();
    size_t p1 = 0, p2 = l - 1;

    // Check for comment
    size_t commentPos = line.find('#');
    if(commentPos != string::npos)
      p2 = commentPos - 1;

    // Trim the string
    while(((line[p1] == ' ') || (line[p1] == '\t')) && (p1 < p2))
      ++ p1;
    while(((line[p2] == ' ') || (line[p2] == '\t')) && (p2 > p1))
      -- p2;

    aResult = line.substr(p1, p2 - p1 + 1);
    if((aResult.size() > 0) || aStream.eof())
      return;
  }
}

// Parse a 3 x float string as a Vector3
static Vector3 ParseVector3(const string aString)
{
  Vector3 result;
  istringstream sstr(aString);
  sstr >> result.x;
  sstr >> result.y;
  sstr >> result.z;
  return result;
}

/// Import a mesh from an OFF file.
void Import_OFF(const char * aFileName, Mesh * aMesh)
{
  // Clear the mesh
  aMesh->Clear();

  // Open the input file
  ifstream f(aFileName, ios_base::in);
  if(f.fail())
    throw runtime_error("Could not open input file.");

  // Some state variables that we need...
  unsigned int numVertices;
  unsigned int numFaces;
  string line;
  istringstream sstr;

  // Read header
  ReadNextLine(f, line);
  if(line != string("OFF"))
    throw runtime_error("Not a valid OFF format file (missing OFF signature).");
  ReadNextLine(f, line);
  sstr.clear();
  sstr.str(line);
  sstr >> numVertices;
  sstr >> numFaces;
  if(numVertices < 1)
    throw runtime_error("Not a valid OFF format file (bad vertex count).");
  if(numFaces < 1)
    throw runtime_error("Not a valid OFF format file (bad face count).");

  // Read vertices
  aMesh->mVertices.resize(numVertices);
  for(unsigned int i = 0; i < numVertices; ++ i)
  {
    ReadNextLine(f, line);
    aMesh->mVertices[i] = ParseVector3(line);
  }

  // Read faces
  list<unsigned int> indices;
  unsigned int idx[3];
  for(unsigned int i = 0; i < numFaces; ++ i)
  {
    ReadNextLine(f, line);
    sstr.clear();
    sstr.str(line);
    int nodeCount;
    sstr >> nodeCount;
    if(nodeCount >= 3)
    {
      sstr >> idx[0];
      sstr >> idx[1];
      sstr >> idx[2];
      nodeCount -= 3;
      while(nodeCount >= 0)
      {
        indices.push_back(idx[0]);
        indices.push_back(idx[1]);
        indices.push_back(idx[2]);
        if(nodeCount > 0)
        {
          idx[1] = idx[2];
          sstr >> idx[2];
        }
        -- nodeCount;
      }
    }
  }

  // Copy triangle indices from the read index list to the mesh index vector
  aMesh->mIndices.resize(indices.size());
  unsigned int j = 0;
  for(list<unsigned int>::iterator i = indices.begin(); i != indices.end(); ++ i)
  {
    aMesh->mIndices[j] = (*i);
    ++ j;
  }

  // Close the input file
  f.close();
}

/// Export a mesh to an OFF file.
void Export_OFF(const char * aFileName, Mesh * aMesh)
{
  // Open the output file
  ofstream f(aFileName, ios_base::out);
  if(f.fail())
    throw runtime_error("Could not open output file.");

  // Mesh information
  unsigned int numVertices = (unsigned int) aMesh->mVertices.size();
  unsigned int numFaces = (unsigned int) aMesh->mIndices.size() / 3;

  // Set floating point precision
  f << setprecision(8);

  // Write herader
  f << "OFF" << endl;
  f << numVertices << " " << numFaces << " 0" << endl;

  // Write vertices
  for(unsigned int i = 0; i < numVertices; ++ i)
    f << aMesh->mVertices[i].x << " " << aMesh->mVertices[i].y << " " << aMesh->mVertices[i].z << endl;

  // Write faces
  for(unsigned int i = 0; i < numFaces; ++ i)
  {
    f << "3 " << aMesh->mIndices[i * 3] << " " <<
                 aMesh->mIndices[i * 3 + 1] << " " <<
                 aMesh->mIndices[i * 3 + 2] << endl;
  }

  // Close the output file
  f.close();
}
