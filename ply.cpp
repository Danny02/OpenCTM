#include <stdexcept>
#include <string>
#include <sstream>
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
Vector3f ParseVertex(string &aString, int aX, int aY, int aZ)
{
  int maxPos = aX;
  if(aY > maxPos) maxPos = aY;
  if(aZ > maxPos) maxPos = aZ;

  Vector3f result;

  istringstream sstr(aString);
  for(int i = 0; i <= maxPos; ++ i)
  {
    float value;
    sstr >> value;
    if(i == aX)
      result.x = value;
    else if(i == aY)
      result.z = value;
    else if(i == aZ)
      result.y = -value;
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
void PLY_Import(istream &aStream, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  // Clear the mesh
  aPoints.clear();
  aIndices.clear();

  // Read header
  unsigned int count, vertexCount = 0, faceCount = 0;
  int xPos = -1, yPos = -1, zPos = -1, propCnt = 0;
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
  aPoints.resize(vertexCount);
  for(unsigned int i = 0; i < vertexCount; ++ i)
  {
    getline(aStream, str);
    aPoints[i] = ParseVertex(str, xPos, yPos, zPos);
  }

  // Read faces
  aIndices.resize(faceCount * 3);
  for(unsigned int i = 0; i < faceCount; ++ i)
  {
    getline(aStream, str);
    int idx1, idx2, idx3;
    ParseFace(str, idx1, idx2, idx3);
    aIndices[i * 3] = idx1;
    aIndices[i * 3 + 1] = idx2;
    aIndices[i * 3 + 2] = idx3;
  }
}

/// Export a PLY file to a stream.
void PLY_Export(ostream &aStream, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  // Write header
  aStream << "ply" << endl;
  aStream << "format ascii 1.0" << endl;
  aStream << "element vertex " << aPoints.size() << endl;
  aStream << "property float x" << endl;
  aStream << "property float y" << endl;
  aStream << "property float z" << endl;
  aStream << "element face " << aIndices.size() / 3 << endl;
  aStream << "property list uchar int vertex_indices" << endl;
  aStream << "end_header" << endl;

  // Write vertices
  for(unsigned int i = 0; i < aPoints.size(); ++ i)
    aStream << aPoints[i].x << " " <<
               aPoints[i].z << " " <<
               -aPoints[i].y << endl;

  // Write faces
  for(unsigned int i = 0; i < aIndices.size() / 3; ++ i)
    aStream << "3 " << aIndices[i * 3] << " " <<
                       aIndices[i * 3 + 1] << " " <<
                       aIndices[i * 3 + 2] << endl;
}
