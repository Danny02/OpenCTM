#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include "ply.h"
#include <openctm.h>

using namespace std;

//-----------------------------------------------------------------------------
// CheckCTMError()
//-----------------------------------------------------------------------------
void CheckCTMError(CTMcontext aContext)
{
  CTMenum err;
  if(err = ctmGetError(aContext))
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
void LoadPLY(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices,
  vector<Vector2f> &aTexCoords, vector<Vector3f> &aNormals)
{
  ifstream fin(aFileName.c_str(), ios_base::in | ios_base::binary);
  if(fin.fail())
    throw runtime_error("Could not open input file.");
  PLY_Import(fin, aPoints, aIndices, aTexCoords, aNormals);
  fin.close();
}

//-----------------------------------------------------------------------------
// SavePLY()
//-----------------------------------------------------------------------------
void SavePLY(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices,
  vector<Vector2f> &aTexCoords, vector<Vector3f> &aNormals)
{
  ofstream fout(aFileName.c_str(), ios_base::out | ios_base::binary);
  if(fout.fail())
    throw runtime_error("Could not open output file.");
  PLY_Export(fout, aPoints, aIndices, aTexCoords, aNormals);
  fout.close();
}

//-----------------------------------------------------------------------------
// LoadCTM()
//-----------------------------------------------------------------------------
void LoadCTM(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices,
  vector<Vector2f> &aTexCoords, vector<Vector3f> &aNormals)
{
  // Import OpenCTM file
  CTMcontext ctm = 0;
  try
  {
    // Create OpenCTM context
    ctm = ctmNewContext(CTM_IMPORT);
    CheckCTMError(ctm);

    // Import file
    CheckCTMError(ctm);
    ctmLoad(ctm, aFileName.c_str());
    CheckCTMError(ctm);

    // Extract mesh info
    CTMuint vertCount = ctmGetInteger(ctm, CTM_VERTEX_COUNT);
    CTMuint triCount = ctmGetInteger(ctm, CTM_TRIANGLE_COUNT);
    CTMuint texMapCount = ctmGetInteger(ctm, CTM_TEX_MAP_COUNT);
    const CTMuint * indices = ctmGetIntegerArray(ctm, CTM_INDICES);
    const CTMfloat * vertices, * normals = 0, * texCoords = 0;
    vertices = ctmGetFloatArray(ctm, CTM_VERTICES);
    if(ctmGetInteger(ctm, CTM_HAS_NORMALS) == CTM_TRUE)
      normals = ctmGetFloatArray(ctm, CTM_NORMALS);
    if(texMapCount > 0)
      texCoords = ctmGetFloatArray(ctm, CTM_TEX_MAP_1);

    CheckCTMError(ctm);

    // Copy index data
    aIndices.resize(triCount * 3);
    for(CTMuint i = 0; i < triCount * 3; ++ i)
      aIndices[i] = indices[i];

    // Copy point data
    aPoints.resize(vertCount);
    for(CTMuint i = 0; i < vertCount; ++ i)
    {
      aPoints[i].x = vertices[i * 3];
      aPoints[i].y = vertices[i * 3 + 1];
      aPoints[i].z = vertices[i * 3 + 2];
    }

    // Copy normal data
    if(normals)
    {
      aNormals.resize(vertCount);
      for(CTMuint i = 0; i < vertCount; ++ i)
      {
        aNormals[i].x = normals[i * 3];
        aNormals[i].y = normals[i * 3 + 1];
        aNormals[i].z = normals[i * 3 + 2];
      }
    }
    else
      aNormals.clear();

    // Copy texture map data
    if(texCoords)
    {
      aTexCoords.resize(vertCount);
      for(CTMuint i = 0; i < vertCount; ++ i)
      {
        aTexCoords[i].x = texCoords[i * 2];
        aTexCoords[i].y = texCoords[i * 2 + 1];
      }
    }
    else
      aTexCoords.clear();

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
void SaveCTM(string &aFileName, vector<Vector3f> &aPoints, vector<int> &aIndices,
  vector<Vector2f> &aTexCoords, vector<Vector3f> &aNormals)
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
    if(aNormals.size() > 0)
      normals = &aNormals[0].x;
    ctmDefineMesh(ctm, (CTMfloat *) &aPoints[0].x, aPoints.size(),
                  (const CTMuint*) &aIndices[0], aIndices.size() / 3,
                  normals);
    CheckCTMError(ctm);

    // Define texture coordinates
    if(aTexCoords.size() > 0)
      ctmAddTexMap(ctm, &aTexCoords[0].x, "Diffuse color", NULL);
    CheckCTMError(ctm);

    // Export file
    ctmCompressionMethod(ctm, CTM_METHOD_MG2);
    CheckCTMError(ctm);
    ctmVertexPrecisionRel(ctm, 0.01);
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
    vector<Vector2f> texCoords;
    vector<Vector3f> normals;

    // Load PLY file
    cout << "Loading " << inFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(inFile));
    if(fileExt == string(".PLY"))
      LoadPLY(inFile, points, indices, texCoords, normals);
    else if(fileExt == string(".CTM"))
      LoadCTM(inFile, points, indices, texCoords, normals);
    else
      throw runtime_error("Unknown input file extension.");

    // Save file
    cout << "Saving " << outFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(outFile));
    if(fileExt == string(".PLY"))
      SavePLY(outFile, points, indices, texCoords, normals);
    else if(fileExt == string(".CTM"))
      SaveCTM(outFile, points, indices, texCoords, normals);
    else
      throw runtime_error("Unknown output file extension.");
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }

  return 0;
}
