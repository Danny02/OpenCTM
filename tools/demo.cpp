#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include "ply.h"
#include "src/openctm.h"

using namespace std;

//-----------------------------------------------------------------------------
// CheckCTMError()
//-----------------------------------------------------------------------------
void CheckCTMError(CTMcontext aContext)
{
  CTMerror err;
  if(err = ctmError(aContext))
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
void LoadPLY(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  ifstream fin(aFileName.c_str(), ios_base::in | ios_base::binary);
  if(fin.fail())
    throw runtime_error("Could not open input file.");
  PLY_Import(fin, aPoints, aIndices);
  fin.close();
}

//-----------------------------------------------------------------------------
// SavePLY()
//-----------------------------------------------------------------------------
void SavePLY(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  ofstream fout(aFileName.c_str(), ios_base::out | ios_base::binary);
  if(fout.fail())
    throw runtime_error("Could not open output file.");
  PLY_Export(fout, aPoints, aIndices);
  fout.close();
}

//-----------------------------------------------------------------------------
// LoadCTM()
//-----------------------------------------------------------------------------
void LoadCTM(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  // Import OpenCTM file
  CTMcontext ctm = 0;
  try
  {
    // Create OpenCTM context
    ctm = ctmNewContext();
    CheckCTMError(ctm);

    // Import file
    CheckCTMError(ctm);
    ctmLoad(ctm, aFileName.c_str());
    CheckCTMError(ctm);

    // Extract mesh
    CTMuint vertCount = ctmGetInteger(ctm, CTM_VERTEX_COUNT);
    CTMuint triCount = ctmGetInteger(ctm, CTM_TRIANGLE_COUNT);
    aPoints.resize(vertCount);
    aIndices.resize(triCount * 3);
    ctmGetMesh(ctm, (CTMfloat *) &aPoints[0].x, aPoints.size(),
               (CTMuint*) &aIndices[0], aIndices.size() / 3, NULL);
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
// SaveCTM()
//-----------------------------------------------------------------------------
void SaveCTM(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices)
{
  // Export OpenCTM file
  CTMcontext ctm = 0;
  try
  {
    // Create OpenCTM context
    ctm = ctmNewContext();
    CheckCTMError(ctm);

    // Define mesh
    ctmDefineMesh(ctm, (CTMfloat *) &aPoints[0].x, aPoints.size(),
                  (const CTMuint*) &aIndices[0], aIndices.size() / 3, NULL);
    CheckCTMError(ctm);

    // Export file
    ctmCompressionMethod(ctm, CTM_METHOD_MG1);
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
    vector<Vector3f> points;
    vector<int> indices;

    // Load PLY file
    cout << "Loading " << inFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(inFile));
    if(fileExt == string(".PLY"))
      LoadPLY(inFile, points, indices);
    else if(fileExt == string(".CTM"))
      LoadCTM(inFile, points, indices);
    else
      throw runtime_error("Unknown input file extension.");

    // Save file
    cout << "Saving " << outFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(outFile));
    if(fileExt == string(".PLY"))
      SavePLY(outFile, points, indices);
    else if(fileExt == string(".CTM"))
      SaveCTM(outFile, points, indices);
    else
      throw runtime_error("Unknown output file extension.");
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }

  return 0;
}
