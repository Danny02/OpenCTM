//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        obj.cpp
// Description: Implementation of the OBJ file format importer/exporter.
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
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include "obj.h"

using namespace std;

// OBJ file face description class (three triangle corners, with one vertex,
// texcoord and normal index each).
class OBJFace {
  public:
    OBJFace()
    {
      for(int i = 0; i < 3; ++ i)
        for(int j = 0; j < 3; ++ j)
          v[i][j] = 0;
    }

    // Contruct a face (one triangle) from an OBJ face description string
    OBJFace(const string aStr)
    {
      // Extract three face corners (one triangle)
      unsigned int pos = 0;
      for(int i = 0; i < 3; ++ i)
      {
        // Extract three /-separated strings (v/vt/vn)
        string v_s[3];
        int j = 0;
        while((pos < aStr.size()) && (aStr[pos] != ' '))
        {
          if(aStr[pos] != '/')
            v_s[j] += aStr[pos];
          else
            ++ j;
          ++ pos;
        }
        ++ pos;

        // Convert the strings to integers
        for(int j = 0; j < 3; ++ j)
        {
          v[i][j] = 0;
          if(v_s[j].size() > 0)
          {
            istringstream ss(v_s[j]);
            ss >> v[i][j];
            if(v[i][j] > 0)
              v[i][j] --;
            else if(v[i][j] < 0)
              throw runtime_error("Negative vertex references in OBJ files are not supported.");
          }
        }
      }
    }

    int v[3][3];  // v[3], vt[3], vn[3]
};

// Parse a 2 x float string as a Vector2
static Vector2 ParseVector2(const string aString)
{
  Vector2 result;
  istringstream sstr(aString);
  sstr >> result.u;
  sstr >> result.v;
  return result;
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

/// Import a mesh from an OBJ file.
void Import_OBJ(const char * aFileName, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();

  // Open the input file
  ifstream f(aFileName, ios_base::in | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open input file.");

  // Mesh description - parsed from the OBJ file
  list<Vector3> vertices;
  list<Vector2> texCoords;
  list<Vector3> normals;
  list<OBJFace> faces;

  // Parse the file
  while(!f.eof())
  {
    string line;
    getline(f, line);
    if(line.size() >= 1)
    {
      if(line.substr(0, 2) == string("v "))
        vertices.push_back(ParseVector3(line.substr(2)));
      else if(line.substr(0, 3) == string("vt "))
        texCoords.push_back(ParseVector2(line.substr(3)));
      else if(line.substr(0, 3) == string("vn "))
        normals.push_back(ParseVector3(line.substr(3)));
      else if(line.substr(0, 2) == string("f "))
        faces.push_back(OBJFace(line.substr(2)));
    }
  }

  // Convert lists to vectors (for random element access)
  vector<Vector3> verticesArray(vertices.begin(), vertices.end());
  vector<Vector2> texCoordsArray(texCoords.begin(), texCoords.end());
  vector<Vector3> normalsArray(normals.begin(), normals.end());

  // Prepare mesh
  aMesh.mVertices.resize(verticesArray.size());
  if(texCoordsArray.size() > 0)
    aMesh.mTexCoords.resize(verticesArray.size());
  if(normalsArray.size() > 0)
    aMesh.mNormals.resize(verticesArray.size());
  aMesh.mIndices.resize(faces.size() * 3);

  // Iterate faces and extract vertex data
  unsigned int idx = 0;
  for(list<OBJFace>::iterator i = faces.begin(); i != faces.end(); ++ i)
  {
    OBJFace &f = (*i);
    aMesh.mIndices[idx ++] = f.v[0][0];
    aMesh.mIndices[idx ++] = f.v[1][0];
    aMesh.mIndices[idx ++] = f.v[2][0];
    aMesh.mVertices[f.v[0][0]] = verticesArray[f.v[0][0]];
    aMesh.mVertices[f.v[1][0]] = verticesArray[f.v[1][0]];
    aMesh.mVertices[f.v[2][0]] = verticesArray[f.v[2][0]];
    if(texCoordsArray.size() > 0)
    {
      aMesh.mTexCoords[f.v[0][0]] = texCoordsArray[f.v[0][1]];
      aMesh.mTexCoords[f.v[1][0]] = texCoordsArray[f.v[1][1]];
      aMesh.mTexCoords[f.v[2][0]] = texCoordsArray[f.v[2][1]];
    }
    if(normalsArray.size() > 0)
    {
      aMesh.mNormals[f.v[0][0]] = normalsArray[f.v[0][2]];
      aMesh.mNormals[f.v[1][0]] = normalsArray[f.v[1][2]];
      aMesh.mNormals[f.v[2][0]] = normalsArray[f.v[2][2]];
    }
  }

  // Close the input file
  f.close();
}

/// Export a mesh to an OBJ file.
void Export_OBJ(const char * aFileName, Mesh &aMesh)
{
  // Open the output file
  ofstream f(aFileName, ios_base::out | ios_base::binary);
  if(f.fail())
    throw runtime_error("Could not open output file.");

  bool hasUVCoords = (aMesh.mTexCoords.size() == aMesh.mVertices.size());
  bool hasNormals = (aMesh.mNormals.size() == aMesh.mVertices.size());

  // Set floating point precision
  f << setprecision(8);

  // Write comment
  if(aMesh.mComment.size() > 0)
    f << "# " << aMesh.mComment << endl;
  f << "# Generator: ctmconv" << endl;

  // Write vertices
  for(unsigned int i = 0; i < aMesh.mVertices.size(); ++ i)
    f << "v " << aMesh.mVertices[i].x << " " << aMesh.mVertices[i].y << " " << aMesh.mVertices[i].z << endl;

  // Write UV coordinates
  if(hasUVCoords)
  {
    for(unsigned int i = 0; i < aMesh.mTexCoords.size(); ++ i)
      f << "vt " << aMesh.mTexCoords[i].u << " " << aMesh.mTexCoords[i].v << endl;
  }

  // Write normals
  if(hasNormals)
  {
    for(unsigned int i = 0; i < aMesh.mNormals.size(); ++ i)
      f << "vn " << aMesh.mNormals[i].x << " " << aMesh.mNormals[i].y << " " << aMesh.mNormals[i].z << endl;
  }

  // Write faces
  unsigned int triCount = aMesh.mIndices.size() / 3;
  f << "s 1" << endl; // Put all faces in the same smoothing group
  for(unsigned int i = 0; i < triCount; ++ i)
  {
    unsigned int idx = aMesh.mIndices[i * 3] + 1;
    f << "f " << idx << "/";
    if(hasUVCoords)
      f << idx;
    f << "/";
    if(hasNormals)
      f << idx;

    idx = aMesh.mIndices[i * 3 + 1] + 1;
    f << " " << idx << "/";
    if(hasUVCoords)
      f << idx;
    f << "/";
    if(hasNormals)
      f << idx;

    idx = aMesh.mIndices[i * 3 + 2] + 1;
    f << " " << idx << "/";
    if(hasUVCoords)
      f << idx;
    f << "/";
    if(hasNormals)
      f << idx;
    f << endl;
  }

  // Close the output file
  f.close();
}
