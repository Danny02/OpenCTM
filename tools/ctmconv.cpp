#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include "ply.h"
#include "mesh.h"
#include <openctm.h>

using namespace std;

//-----------------------------------------------------------------------------
// CheckCTMError()
//-----------------------------------------------------------------------------
void CheckCTMError(CTMcontext aContext)
{
  CTMenum err = ctmGetError(aContext);
  if(err != CTM_NONE)
  {
    stringstream ss;
    ss << "CTM failed with error code " << err;
    throw runtime_error(ss.str());
  }
}

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
// LoadPLY()
//-----------------------------------------------------------------------------
void LoadPLY(string &aFileName, Mesh &aMesh)
{
  ifstream fin(aFileName.c_str(), ios_base::in | ios_base::binary);
  if(fin.fail())
    throw runtime_error("Could not open input file.");
  PLY_Import(fin, aMesh);
  fin.close();
}

//-----------------------------------------------------------------------------
// SavePLY()
//-----------------------------------------------------------------------------
void SavePLY(string &aFileName, Mesh &aMesh)
{
  ofstream fout(aFileName.c_str(), ios_base::out | ios_base::binary);
  if(fout.fail())
    throw runtime_error("Could not open output file.");
  PLY_Export(fout, aMesh);
  fout.close();
}

//-----------------------------------------------------------------------------
// LoadCTM()
//-----------------------------------------------------------------------------
void LoadCTM(string &aFileName, Mesh &aMesh)
{
  // Import OpenCTM file
  aMesh.LoadFromFile(aFileName.c_str());
}

//-----------------------------------------------------------------------------
// SaveCTM()
//-----------------------------------------------------------------------------
void SaveCTM(string &aFileName, Mesh &aMesh)
{
  // Export OpenCTM file
  CTMcontext ctm = 0;
  try
  {
    // Create OpenCTM context
    ctm = ctmNewContext(CTM_EXPORT);
    CheckCTMError(ctm);

    // Define mesh
    CTMfloat * normals = 0;
    if(aMesh.mNormals.size() > 0)
      normals = &aMesh.mNormals[0].x;
    ctmDefineMesh(ctm, (CTMfloat *) &aMesh.mVertices[0].x, aMesh.mVertices.size(),
                  (const CTMuint*) &aMesh.mIndices[0], aMesh.mIndices.size() / 3,
                  normals);
    CheckCTMError(ctm);

    // Define texture coordinates
    if(aMesh.mTexCoords.size() > 0)
      ctmAddTexMap(ctm, &aMesh.mTexCoords[0].u, "Diffuse color", NULL);
    CheckCTMError(ctm);

    // Export file
    ctmCompressionMethod(ctm, CTM_METHOD_MG2);
    CheckCTMError(ctm);
    ctmVertexPrecisionRel(ctm, 0.01f);
    CheckCTMError(ctm);
    ctmSave(ctm, aFileName.c_str());
    CheckCTMError(ctm);

    // Free OpenCTM context
    ctmFreeContext(ctm);
    ctm = 0;
  }
  catch(exception &e)
  {
    if(ctm)
      ctmFreeContext(ctm);
    throw;
  }
}


//-----------------------------------------------------------------------------
// main()
//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
  // Get file names
  if(argc < 3)
  {
    cout << "Usage: " << argv[0] << " infile outfile" << endl;
    return 0;
  }
  string inFile(argv[1]);
  string outFile(argv[2]);

  try
  {
    string fileExt;

    // Define mesh
    Mesh mesh;

    // Load PLY file
    cout << "Loading " << inFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(inFile));
    if(fileExt == string(".PLY"))
      LoadPLY(inFile, mesh);
    else if(fileExt == string(".CTM"))
      LoadCTM(inFile, mesh);
    else
      throw runtime_error("Unknown input file extension.");

    // Save file
    cout << "Saving " << outFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(outFile));
    if(fileExt == string(".PLY"))
      SavePLY(outFile, mesh);
    else if(fileExt == string(".CTM"))
      SaveCTM(outFile, mesh);
    else
      throw runtime_error("Unknown output file extension.");
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }

  return 0;
}
