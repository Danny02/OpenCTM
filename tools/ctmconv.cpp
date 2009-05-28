#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include "convoptions.h"
#include "mesh.h"
#include "ply.h"
#include "stl.h"
#include <openctm.h>

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
// LoadSTL()
//-----------------------------------------------------------------------------
void LoadSTL(string &aFileName, Mesh &aMesh)
{
  ifstream fin(aFileName.c_str(), ios_base::in | ios_base::binary);
  if(fin.fail())
    throw runtime_error("Could not open input file.");
  STL_Import(fin, aMesh);
  fin.close();
}

//-----------------------------------------------------------------------------
// SaveSTL()
//-----------------------------------------------------------------------------
void SaveSTL(string &aFileName, Mesh &aMesh)
{
  ofstream fout(aFileName.c_str(), ios_base::out | ios_base::binary);
  if(fout.fail())
    throw runtime_error("Could not open output file.");
  STL_Export(fout, aMesh);
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
void SaveCTM(string &aFileName, Mesh &aMesh, Options &aOptions)
{
  // Export OpenCTM file
  aMesh.SaveToFile(aFileName.c_str(), aOptions);
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
    cout << "  --method arg    Select compression method (RAW, MG1, MG2)" << endl;
    cout << "  --vprec arg     Set vertex precision" << endl;
    cout << "  --vprecrel arg  Set vertex precision, relative method" << endl;
    cout << "  --nprec arg     Set normal precision" << endl;
    cout << "  --tprec arg     Set texture map precision" << endl;
    cout << "  --cprec arg     Set color precision" << endl;
    cout << "  --upaxis arg    Set up axis (X, Y, Z, -X, -Y, -Z). If != Z, the mesh will" << endl;
    cout << "                  be flipped." << endl;
    return 0;
  }

  try
  {
    string fileExt;

    // Define mesh
    Mesh mesh;

    // Load input file
    cout << "Loading " << inFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(inFile));
    if(fileExt == string(".PLY"))
      LoadPLY(inFile, mesh);
    else if(fileExt == string(".STL"))
      LoadSTL(inFile, mesh);
    else if(fileExt == string(".CTM"))
      LoadCTM(inFile, mesh);
    else
      throw runtime_error("Unknown input file extension.");

    // Save output file
    cout << "Saving " << outFile << "..." << endl;
    fileExt = UpperCase(ExtractFileExt(outFile));
    if(fileExt == string(".PLY"))
      SavePLY(outFile, mesh);
    else if(fileExt == string(".STL"))
      SaveSTL(outFile, mesh);
    else if(fileExt == string(".CTM"))
      SaveCTM(outFile, mesh, opt);
    else
      throw runtime_error("Unknown output file extension.");
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }

  return 0;
}
