#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <cctype>
#include "convoptions.h"
#include "mesh.h"
#include "ctm.h"
#include "ply.h"
#include "stl.h"
#include "3ds.h"

using namespace std;


//-----------------------------------------------------------------------------
// UpperCase()
//-----------------------------------------------------------------------------
string UpperCase(const string &aString)
{
  string result(aString);
  for(unsigned int i = 0; i < result.size(); ++ i)
    result[i] = toupper(result[i]);
  return result;
}

//-----------------------------------------------------------------------------
// ExtractFileExt()
//-----------------------------------------------------------------------------
string ExtractFileExt(const string &aString)
{
  string result = "";
  size_t extPos = aString.rfind(".");
  if(extPos != string::npos)
    result = aString.substr(extPos);
  return result;
}

//-----------------------------------------------------------------------------
// PreProcessMesh()
//-----------------------------------------------------------------------------
void PreProcessMesh(Mesh &aMesh, Options &aOptions)
{
  // Nothing to do?
  if((aOptions.mScale == 1.0f) && (aOptions.mUpAxis == uaZ))
    return;

  // Create 3x3 transformation matrices for the vertices and the normals
  Vector3 vX, vY, vZ;
  Vector3 nX, nY, nZ;
  switch(aOptions.mUpAxis)
  {
    case uaX:
      nX = Vector3(0.0f, 0.0f, 1.0f);
      nY = Vector3(0.0f, 1.0f, 0.0f);
      nZ = Vector3(-1.0f, 0.0f, 0.0f);
      break;
    case uaY:
      nX = Vector3(1.0f, 0.0f, 0.0f);
      nY = Vector3(0.0f, 0.0f, 1.0f);
      nZ = Vector3(0.0f, -1.0f, 0.0f);
      break;
    case uaZ:
      nX = Vector3(1.0f, 0.0f, 0.0f);
      nY = Vector3(0.0f, 1.0f, 0.0f);
      nZ = Vector3(0.0f, 0.0f, 1.0f);
      break;
    case uaNX:
      nX = Vector3(0.0f, 0.0f, -1.0f);
      nY = Vector3(0.0f, 1.0f, 0.0f);
      nZ = Vector3(1.0f, 0.0f, 0.0f);
      break;
    case uaNY:
      nX = Vector3(1.0f, 0.0f, 0.0f);
      nY = Vector3(0.0f, 0.0f, -1.0f);
      nZ = Vector3(0.0f, 1.0f, 0.0f);
      break;
    case uaNZ:
      nX = Vector3(-1.0f, 0.0f, 0.0f);
      nY = Vector3(0.0f, 1.0f, 0.0f);
      nZ = Vector3(0.0f, 0.0f, -1.0f);
      break;
  }
  vX = nX * aOptions.mScale;
  vY = nY * aOptions.mScale;
  vZ = nZ * aOptions.mScale;

  cout << "Processing..." << endl;

  // Update all vertex coordinates
  for(CTMuint i = 0; i < aMesh.mVertices.size(); ++ i)
    aMesh.mVertices[i] = vX * aMesh.mVertices[i].x +
                         vY * aMesh.mVertices[i].y +
                         vZ * aMesh.mVertices[i].z;

  // Update all normals
  for(CTMuint i = 0; i < aMesh.mNormals.size(); ++ i)
    aMesh.mNormals[i] = nX * aMesh.mNormals[i].x +
                        nY * aMesh.mNormals[i].y +
                        nZ * aMesh.mNormals[i].z;
}


//-----------------------------------------------------------------------------
// main()
//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
  // Get file names and options
  Options opt;
  string inFile;
  string outFile;
  try
  {
    if(argc < 3)
      throw runtime_error("Too few arguments.");
    inFile = string(argv[1]);
    outFile = string(argv[2]);
    opt.GetFromArgs(argc, argv, 3);
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl << endl;
    cout << "Usage: " << argv[0] << " infile outfile [options]" << endl << endl;
    cout << "Options:" << endl;
    cout << endl << " Data manipulation (all formats)" << endl;
    cout << "  --scale arg     Scale the mesh by a scalar factor." << endl;
    cout << "  --upaxis arg    Set up axis (X, Y, Z, -X, -Y, -Z). If != Z, the mesh will" << endl;
    cout << "                  be flipped." << endl;
    cout << endl << " OpenCTM output" << endl;
    cout << "  --method arg    Select compression method (RAW, MG1, MG2)" << endl;
    cout << endl << " OpenCTM MG2 method" << endl;
    cout << "  --vprec arg     Set vertex precision" << endl;
    cout << "  --vprecrel arg  Set vertex precision, relative method" << endl;
    cout << "  --nprec arg     Set normal precision" << endl;
    cout << "  --tprec arg     Set texture map precision" << endl;
    cout << "  --cprec arg     Set color precision" << endl;
    cout << endl << " Miscellaneous" << endl;
    cout << "  --comment arg   Set the file comment (default is to use the comment" << endl;
    cout << "                  from the input file, if any)." << endl;
    cout << "  --texfile arg   Set the texture file name reference for the texture" << endl;
    cout << "                  (default is to use the texture file name reference" << endl;
    cout << "                  from the input file, if any)." << endl;
    return 0;
  }

  try
  {
    // Define mesh
    Mesh mesh;

    // Load input file
    cout << "Loading " << inFile << "..." << endl;
    string fileExt = UpperCase(ExtractFileExt(inFile));
    if(fileExt == string(".PLY"))
      Import_PLY(inFile.c_str(), mesh);
    else if(fileExt == string(".STL"))
      Import_STL(inFile.c_str(), mesh);
    else if(fileExt == string(".3DS"))
      Import_3DS(inFile.c_str(), mesh);
    else if(fileExt == string(".CTM"))
      Import_CTM(inFile.c_str(), mesh);
    else
      throw runtime_error("Unknown input file extension.");

    // Manipulate the mesh
    PreProcessMesh(mesh, opt);

    // Override comment?
    if(opt.mComment.size() > 0)
      mesh.mComment = opt.mComment;

    // Override texture file name?
    if(opt.mTexFileName.size() > 0)
      mesh.mTexFileName = opt.mTexFileName;

    // Save output file
    cout << "Saving " << outFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(outFile));
    if(fileExt == string(".PLY"))
      Export_PLY(outFile.c_str(), mesh);
    else if(fileExt == string(".STL"))
      Export_STL(outFile.c_str(), mesh);
    else if(fileExt == string(".3DS"))
      Export_3DS(outFile.c_str(), mesh);
    else if(fileExt == string(".CTM"))
      Export_CTM(outFile.c_str(), mesh, opt);
    else
      throw runtime_error("Unknown output file extension.");
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }

  return 0;
}
