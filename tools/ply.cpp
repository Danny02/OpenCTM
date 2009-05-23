#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include "ply.h"

using namespace std;

/// Parse an "element" string.
static string ParseElement(string &aLine, unsigned int &aCount)
{
  // Get property name
  string propName = aLine.substr(8);

  // Get property count
  size_t pos = propName.find(" ");
  if(pos == string::npos)
    throw runtime_error("Invalid PLY property description.");
  string countStr = propName.substr(pos + 1);
  istringstream sstr(countStr);
  sstr >> aCount;

  return propName.substr(0, pos);
}

/// Parse a vertex string.
Vector3 ParseVertex(string &aString, int aX, int aY, int aZ)
{
  int maxPos = aX;
  if(aY > maxPos) maxPos = aY;
  if(aZ > maxPos) maxPos = aZ;

  Vector3 result;

  istringstream sstr(aString);
  for(int i = 0; i <= maxPos; ++ i)
  {
    float value;
    sstr >> value;
    if(i == aX)
      result.x = value;
    else if(i == aY)
      result.y = value;
    else if(i == aZ)
      result.z = value;
  }

  return result;
}

/// Parse texture coordinates from a vertex string.
Vector2 ParseTexCoord(string &aString, int aS, int aT)
{
  int maxPos = aS;
  if(aT > maxPos) maxPos = aT;

  Vector2 result;

  istringstream sstr(aString);
  for(int i = 0; i <= maxPos; ++ i)
  {
    float value;
    sstr >> value;
    if(i == aS)
      result.u = value;
    else if(i == aT)
      result.v = value;
  }

  return result;
}

/// Parse normals from a vertex string.
Vector3 ParseNormal(string &aString, int aNX, int aNY, int aNZ)
{
  int maxPos = aNX;
  if(aNY > maxPos) maxPos = aNY;
  if(aNZ > maxPos) maxPos = aNZ;

  Vector3 result;

  istringstream sstr(aString);
  for(int i = 0; i <= maxPos; ++ i)
  {
    float value;
    sstr >> value;
    if(i == aNX)
      result.x = value;
    else if(i == aNY)
      result.y = value;
    else if(i == aNZ)
      result.z = value;
  }

  return result;
}

/// Parse a face string.
void ParseFace(string &aString, int &aIdx1, int &aIdx2, int &aIdx3)
{
  istringstream sstr(aString);
  int listCount;
  sstr >> listCount;
  if(listCount != 3)
    throw runtime_error("Unsupported PLY face description (only triangles are supported).");
  sstr >> aIdx1;
  sstr >> aIdx2;
  sstr >> aIdx3;
}

/// Import a PLY file from a stream.
void PLY_Import(istream &aStream, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();

  // Read header
  unsigned int count, vertexCount = 0, faceCount = 0;
  int xPos = -1, yPos = -1, zPos = -1, sPos = -1, tPos = -1, nxPos = -1, nyPos = -1, nzPos = -1, propCnt = 0;
  string elementType("");
  string str;
  getline(aStream, str);
  if(str != string("ply"))
    throw runtime_error("Not a PLY format file.");
  getline(aStream, str);
  if(str != string("format ascii 1.0"))
    throw runtime_error("Not an ASCII 1.0 PLY format file.");
  do
  {
    // Get next headr line
    getline(aStream, str);

    if(str.substr(0, 7) == string("element"))
    {
      // This is the start of a new element description
      elementType = ParseElement(str, count);
      if(elementType == string("vertex"))
        vertexCount = count;
      else if(elementType == string("face"))
        faceCount = count;
      propCnt = 0;
    }
    else if(str.substr(0, 8) == string("property"))
    {
      // This is a property of an element description
      string propDescr = str.substr(9);
      if(elementType == string("vertex"))
      {
        // Find the position of x, y and z
        size_t pos = propDescr.find(" ");
        if(pos == string::npos)
          throw runtime_error("Unsupported vertex description property.");
        string porpName = propDescr.substr(pos + 1);
        if(porpName == string("x"))
          xPos = propCnt;
        else if(porpName == string("y"))
          yPos = propCnt;
        else if(porpName == string("z"))
          zPos = propCnt;
        else if(porpName == string("s"))
          sPos = propCnt;
        else if(porpName == string("t"))
          tPos = propCnt;
        else if(porpName == string("nx"))
          nxPos = propCnt;
        else if(porpName == string("ny"))
          nyPos = propCnt;
        else if(porpName == string("nz"))
          nzPos = propCnt;
      }
      else if(elementType == string("face"))
      {
        // We only support a single face description type right now...
        if((propDescr != string("list uint8 int32 vertex_indices")) &&
           (propDescr != string("list char int vertex_indices")) &&
           (propDescr != string("list char uint vertex_indices")) &&
           (propDescr != string("list uchar int vertex_indices")) &&
           (propDescr != string("list uchar uint vertex_indices")))
          throw runtime_error("Unsupported face description property.");
      }

      ++ propCnt;
    }
  }
  while((str != string("end_header")) && !aStream.eof());

  // End of file?
  if(((vertexCount > 0) || (faceCount > 0)) && aStream.eof())
    throw runtime_error("Premature end of PLY file.");

  // Did we get a proper vertex description?
  if((xPos < 0) || (yPos < 0) || (zPos < 0))
    throw runtime_error("Incomplete PLY vertex description format (need x, y and z).");

  // Read vertices
  aMesh.mVertices.resize(vertexCount);
  if(sPos >= 0)
    aMesh.mTexCoords.resize(vertexCount);
  if(nxPos >= 0)
    aMesh.mNormals.resize(vertexCount);
  for(unsigned int i = 0; i < vertexCount; ++ i)
  {
    getline(aStream, str);
    aMesh.mVertices[i] = ParseVertex(str, xPos, yPos, zPos);
    if(sPos >= 0)
      aMesh.mTexCoords[i] = ParseTexCoord(str, sPos, tPos);
    if(nxPos >= 0)
      aMesh.mNormals[i] = ParseNormal(str, nxPos, nyPos, nzPos);
  }

  // Read faces
  aMesh.mIndices.resize(faceCount * 3);
  for(unsigned int i = 0; i < faceCount; ++ i)
  {
    getline(aStream, str);
    int idx1, idx2, idx3;
    ParseFace(str, idx1, idx2, idx3);
    aMesh.mIndices[i * 3] = idx1;
    aMesh.mIndices[i * 3 + 1] = idx2;
    aMesh.mIndices[i * 3 + 2] = idx3;
  }
}

/// Export a PLY file to a stream.
void PLY_Export(ostream &aStream, Mesh &aMesh)
{
  // Write header
  aStream << "ply" << endl;
  aStream << "format ascii 1.0" << endl;
  if(aMesh.mComment.size() > 0)
    aStream << "comment " << aMesh.mComment << endl;
  aStream << "element vertex " << aMesh.mVertices.size() << endl;
  aStream << "property float x" << endl;
  aStream << "property float y" << endl;
  aStream << "property float z" << endl;
  if(aMesh.mTexCoords.size() > 0)
  {
    aStream << "property float s" << endl;
    aStream << "property float t" << endl;
  }
  if(aMesh.mNormals.size() > 0)
  {
    aStream << "property float nx" << endl;
    aStream << "property float ny" << endl;
    aStream << "property float nz" << endl;
  }
  aStream << "element face " << aMesh.mIndices.size() / 3 << endl;
  aStream << "property list uchar int vertex_indices" << endl;
  aStream << "end_header" << endl;

  // Write vertices
  for(unsigned int i = 0; i < aMesh.mVertices.size(); ++ i)
  {
    aStream << aMesh.mVertices[i].x << " " <<
               aMesh.mVertices[i].y << " " <<
               aMesh.mVertices[i].z;
    if(aMesh.mTexCoords.size() > 0)
      aStream << " " << aMesh.mTexCoords[i].u << " " <<
                        aMesh.mTexCoords[i].v;
    if(aMesh.mNormals.size() > 0)
      aStream << " " << aMesh.mNormals[i].x << " " <<
                        aMesh.mNormals[i].y << " " <<
                        aMesh.mNormals[i].z;
    aStream << endl;
  }

  // Write faces
  for(unsigned int i = 0; i < aMesh.mIndices.size() / 3; ++ i)
    aStream << "3 " << aMesh.mIndices[i * 3] << " " <<
                       aMesh.mIndices[i * 3 + 1] << " " <<
                       aMesh.mIndices[i * 3 + 2] << endl;
}
