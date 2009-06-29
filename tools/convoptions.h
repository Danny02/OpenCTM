#ifndef __CONVOPTIONS_H_
#define __CONVOPTIONS_H_

#include <string>
#include <openctm.h>


typedef enum {
  uaX, uaY, uaZ, uaNX, uaNY, uaNZ
} UpAxis;

class Options {
  public:
    /// Constructor
    Options();

    /// Get options from the command line arguments
    void GetFromArgs(int argc, char **argv, int aStartIdx);

    CTMfloat mScale;
    UpAxis mUpAxis;

    CTMenum mMethod;

    CTMfloat mVertexPrecision;
    CTMfloat mVertexPrecisionRel;
    CTMfloat mNormalPrecision;
    CTMfloat mTexMapPrecision;
    CTMfloat mColorPrecision;

    std::string mComment;
    std::string mTexFileName;
};

#endif // __CONVOPTIONS_H_
