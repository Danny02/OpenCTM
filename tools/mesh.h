#ifndef __MESH_H_
#define __MESH_H_

#include <vector>
#include <string>
#include <cmath>

class Vector2 {
  public:
    Vector2()
    {
      u = 0.0f; v = 0.0f;
    }

    Vector2(float a, float b)
    {
      u = a; v = b;
    }

    Vector2(const Vector2 &a)
    {
      u = a.u; v = a.v;
    }

    float u, v;
};

class Vector3 {
  public:
    Vector3()
    {
      x = 0.0f; y = 0.0f; z = 0.0f;
    }

    Vector3(float a, float b, float c)
    {
      x = a; y = b; z = c;
    }

    Vector3(const Vector3 &a)
    {
      x = a.x; y = a.y; z = a.z;
    }

    inline Vector3 operator+(const Vector3 &v) const 
    {
      return Vector3(x + v.x,  y + v.y,  z + v.z);
    }

    inline Vector3 operator-(const Vector3 &v) const 
    {
      return Vector3(x - v.x,  y - v.y,  z - v.z);
    }

    inline Vector3 operator*(const float &aScale) const 
    {
      return Vector3(aScale * x, aScale * y, aScale * z);
    }

    inline void operator+=(const Vector3 &v) 
    {
      x += v.x;
      y += v.y;
      z += v.z;
    }

    float Abs()
    {
      return sqrtf(x * x + y * y + z * z);
    }

    float x, y, z;
};

class Vector4 {
  public:
    Vector4()
    {
      x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
    }

    Vector4(float a, float b, float c, float d)
    {
      x = a; y = b; z = c; w = d;
    }

    Vector4(const Vector4 &a)
    {
      x = a.x; y = a.y; z = a.z; w = a.w;
    }

    float x, y, z, w;
};


class Mesh {
  public:
    /// Clear the mesh
    void Clear();

    /// Load the mesh from a file
    void LoadFromFile(const char * aFileName);

    /// Save the mesh to a file
    void SaveToFile(const char * aFileName);

    /// Calculate smooth per-vertex normals
    void CalculateNormals();

    /// Calculate the bounding box for the mesh
    void BoundingBox(Vector3 &aMin, Vector3 &aMax);

    std::string mComment;
    std::vector<int> mIndices;
    std::vector<Vector3> mVertices;
    std::vector<Vector3> mNormals;
    std::vector<Vector4> mColors;
    std::vector<Vector2> mTexCoords;
};


#endif // __MESH_H_
