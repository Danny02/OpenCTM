#include <stdexcept>
#include <string>
#include <sstream>
#include "convoptions.h"

using namespace std;


/// Constructor
Options::Options()
{
  // Set default values
  mUpAxis = uaZ;
  mMethod = CTM_METHOD_MG2;
  mVertexPrecision = 0.0f;
  mVertexPrecisionRel = 0.01f;
  mNormalPrecision = 1.0f / 256.0f;
  mTexMapPrecision = 1.0f / 4096.0f;
  mColorPrecision = 1.0f / 256.0f;
}

/// Convert a string to a floating point value
static CTMfloat GetFloatArg(char * aFloatString)
{
  stringstream s;
  s << aFloatString;
  s.seekg(0);
  CTMfloat f;
  s >> f;
  return f;
}

/// Get options from the command line arguments
void Options::GetFromArgs(int argc, char **argv, int aStartIdx)
{
  for(int i = aStartIdx; i < argc; ++ i)
  {
    string cmd(argv[i]);
    if((cmd == string("--upaxis")) && (i < (argc - 1)))
    {
      string upaxis(argv[i + 1]);
      ++ i;
      if(upaxis == string("X"))
        mUpAxis = uaX;
      else if(upaxis == string("Y"))
        mUpAxis = uaY;
      else if(upaxis == string("Z"))
        mUpAxis = uaZ;
      else if(upaxis == string("-X"))
        mUpAxis = uaNX;
      else if(upaxis == string("-Y"))
        mUpAxis = uaNY;
      else if(upaxis == string("-Z"))
        mUpAxis = uaNZ;
      else
        throw runtime_error("Invalid up axis (use X, Y, Z, -X, -Y or -Z).");
    }
    else if((cmd == string("--method")) && (i < (argc - 1)))
    {
      string method(argv[i + 1]);
      ++ i;
      if(method == string("RAW"))
        mMethod = CTM_METHOD_RAW;
      else if(method == string("MG1"))
        mMethod = CTM_METHOD_MG1;
      else if(method == string("MG2"))
        mMethod = CTM_METHOD_MG2;
      else
        throw runtime_error("Invalid method (use RAW, MG1 or MG2).");
    }
    else if((cmd == string("--vprec")) && (i < (argc - 1)))
    {
      mVertexPrecision = GetFloatArg(argv[i + 1]);
      ++ i;
    }
    else if((cmd == string("--vprecrel")) && (i < (argc - 1)))
    {
      mVertexPrecisionRel = GetFloatArg(argv[i + 1]);
      ++ i;
    }
    else if((cmd == string("--nprec")) && (i < (argc - 1)))
    {
      mNormalPrecision = GetFloatArg(argv[i + 1]);
      ++ i;
    }
    else if((cmd == string("--tprec")) && (i < (argc - 1)))
    {
      mTexMapPrecision = GetFloatArg(argv[i + 1]);
      ++ i;
    }
    else if((cmd == string("--cprec")) && (i < (argc - 1)))
    {
      mColorPrecision = GetFloatArg(argv[i + 1]);
      ++ i;
    }
    else
      throw runtime_error(string("Invalid argument: ") + cmd);
  }
}
