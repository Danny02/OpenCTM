#ifndef __PLY_H_
#define __PLY_H_

#include <istream>
#include <ostream>
#include <vector>

/// Minimalist 2D vector class.
class Vector2f
{
  public:
    Vector2f()
    {
      x = y = 0.0;
    }

    float x, y;
};

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
void PLY_Import(std::istream &aStream, std::vector<Vector3f> &aPoints, std::vector<int> &aIndices,
  std::vector<Vector2f> &aTexCoords);

/// Export a PLY file to a stream.
void PLY_Export(std::ostream &aStream, std::vector<Vector3f> &aPoints, std::vector<int> &aIndices,
  std::vector<Vector2f> &aTexCoords);

#endif // __PLY_H_
