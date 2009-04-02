#ifndef __PLY_H_
#define __PLY_H_

#include <istream>
#include <ostream>
#include <vector>

/// Minimalist 3D vector class.
class Vector3f
{
  public:
    Vector3f()
    {
      x = y = z = 0.0;
    }

    float x, y, z;
};

/// Import a PLY file to a stream.
void PLY_Import(std::istream &aStream, std::vector<Vector3f> &aPoints, std::vector<int> &aIndices);

/// Export a PLY file to a stream.
void PLY_Export(std::ostream &aStream, std::vector<Vector3f> &aPoints, std::vector<int> &aIndices);

#endif // __PLY_H_
