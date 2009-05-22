#include <stdexcept>
#include <openctm.h>
#include "mesh.h"

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


using namespace std;

/// Clear the mesh
void Mesh::Clear()
{
  mComment = string("");
  mIndices.clear();
  mVertices.clear();
  mNormals.clear();
  mColors.clear();
  mTexCoords.clear();
}

/// Calculate smooth per-vertex normals
void Mesh::CalculateNormals()
{
  // Calculate the flat normals
  vector<Vector3> flatNormals;
  flatNormals.resize(mIndices.size() / 3);

  // Clear the smooth normals
  mNormals.resize(mVertices.size());
  for(unsigned int i = 0; i < mNormals.size(); ++ i)
    mNormals[i] = Vector3(0.0f, 0.0f, 0.0f);
}

/// Load the mesh from a file
void Mesh::LoadFromFile(const char * aFileName)
{
  Clear();

  // Load the file using the OpenCTM API
  CTMcontext ctm = ctmNewContext(CTM_IMPORT);
  try
  {
    // Load the file
    ctmLoad(ctm, aFileName);
    if(ctmGetError(ctm) != CTM_NONE)
      throw runtime_error("Unable to load the file.");

    // Extract indices
    CTMuint numTriangles = ctmGetInteger(ctm, CTM_TRIANGLE_COUNT);
    mIndices.resize(numTriangles * 3);
    const CTMuint * indices = ctmGetIntegerArray(ctm, CTM_INDICES);
    for(CTMuint i = 0; i < numTriangles * 3; ++ i)
      mIndices[i] = indices[i];

    // Extract vertices
    CTMuint numVertices = ctmGetInteger(ctm, CTM_VERTEX_COUNT);
    mVertices.resize(numVertices);
    const CTMfloat * vertices = ctmGetFloatArray(ctm, CTM_VERTICES);
    for(CTMuint i = 0; i < numVertices; ++ i)
    {
      mVertices[i].x = vertices[i * 3];
      mVertices[i].y = vertices[i * 3 + 1];
      mVertices[i].z = vertices[i * 3 + 2];
    }

    // Extract normals
    if(ctmGetInteger(ctm, CTM_HAS_NORMALS) == CTM_TRUE)
    {
      mNormals.resize(numVertices);
      const CTMfloat * normals = ctmGetFloatArray(ctm, CTM_NORMALS);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mNormals[i].x = normals[i * 3];
        mNormals[i].y = normals[i * 3 + 1];
        mNormals[i].z = normals[i * 3 + 2];
      }
    }

    // Extract texture coordinates
    if(ctmGetInteger(ctm, CTM_TEX_MAP_COUNT) > 0)
    {
      mTexCoords.resize(numVertices);
      const CTMfloat * texCoords = ctmGetFloatArray(ctm, CTM_TEX_MAP_1);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mTexCoords[i].u = texCoords[i * 2];
        mTexCoords[i].v = texCoords[i * 2 + 1];
      }
    }

    // Extract colors
    CTMenum colorAttrib = ctmGetNamedAttribMap(ctm, "Colors");
    if(colorAttrib != CTM_NONE)
    {
      mColors.resize(numVertices);
      const CTMfloat * colors = ctmGetFloatArray(ctm, colorAttrib);
      for(CTMuint i = 0; i < numVertices; ++ i)
      {
        mColors[i].x = colors[i * 4];
        mColors[i].y = colors[i * 4 + 1];
        mColors[i].z = colors[i * 4 + 2];
        mColors[i].w = colors[i * 4 + 3];
      }
    }

    // We are done with the OpenCTM context
    ctmFreeContext(ctm);
  }
  catch(exception &e)
  {
    ctmFreeContext(ctm);
    throw;
  }
}

/// Save the mesh to a file
void Mesh::SaveToFile(const char * aFileName)
{
}

/// Draw the mesh to OpenGL
void Mesh::Draw()
{
  // We always have vertices
  glVertexPointer(3, GL_FLOAT, 0, &mVertices[0]);
  glEnableClientState(GL_VERTEX_ARRAY);

  // Do we have normals?
  if(mNormals.size() == mVertices.size())
  {
    glNormalPointer(GL_FLOAT, 0, &mNormals[0]);
    glEnableClientState(GL_NORMAL_ARRAY);
  }

  // Do we have texture coordinates?
  if(mTexCoords.size() == mVertices.size())
  {
    glTexCoordPointer(2, GL_FLOAT, 0, &mTexCoords[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  // Do we have colors?
  if(mColors.size() == mVertices.size())
  {
    glColorPointer(4, GL_FLOAT, 0, &mColors[0]);
    glEnableClientState(GL_COLOR_ARRAY);
  }

  // Use glDrawElements to draw the triangles...
  glShadeModel(GL_SMOOTH);
  glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT,
                 &mIndices[0]);

  // We do not use the client state anymore...
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}
