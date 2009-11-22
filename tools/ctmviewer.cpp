//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        ctmviewer.cpp
// Description: 3D file viewer. The program can be used to view OpenCTM files
//              in an interactive OpenGL window. Files in other supported
//              formats can also be viewed.
//-----------------------------------------------------------------------------
// Copyright (c) 2009 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <GL/glew.h>
#ifdef __APPLE_CC__
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif
#include <jpeglib.h>
#include <openctm.h>
#include "mesh.h"
#include "meshio.h"
#include "sysdialog.h"

using namespace std;


// We need PI
#ifndef PI
  #define PI 3.141592653589793238462643f
#endif


//-----------------------------------------------------------------------------
// GLSL source code (generated from source by bin2c)
//-----------------------------------------------------------------------------

#include "phong_vert.h"
#include "phong_frag.h"


//-----------------------------------------------------------------------------
// A class for OpenGL rendered GUI buttons
//-----------------------------------------------------------------------------

class GLButton {

private:
  GLuint mTexHandle;

public:
  /// Constructor.
  GLButton()
  {
    mTexHandle = 0;
    mPosX = 0;
    mPosY = 0;
    mWidth = 32;
    mHeight = 32;
  }

  /// Destructor.
  ~GLButton()
  {
  }

  GLint mPosX, mPosY;
  GLint mWidth, mHeight;
};


//-----------------------------------------------------------------------------
// GLUT callback function prototypes
//-----------------------------------------------------------------------------

void GLUTWindowRedraw(void);
void GLUTWindowResize(int w, int h);
void GLUTMouseClick(int button, int state, int x, int y);
void GLUTMouseMove(int x, int y);
void GLUTKeyDown(unsigned char key, int x, int y);
void GLUTSpecialKeyDown(int key, int x, int y);


//-----------------------------------------------------------------------------
// The GLViewer application class
//-----------------------------------------------------------------------------

class GLViewer {

private:

  //-----------------------------------------------------------------------------
  // State variables
  //-----------------------------------------------------------------------------

  string mFileName, mFilePath;
  long mFileSize;

  int mWidth, mHeight;
  int mOldMouseX, mOldMouseY;
  bool mMouseRotate;
  bool mMouseZoom;

  Mesh mMesh;
  Vector3 mAABBMin, mAABBMax;
  Vector3 mCameraPosition, mCameraLookAt;
  GLuint mDisplayList;
  GLenum mPolyMode;

  GLuint mTexHandle;

  bool mUseShader;
  GLuint mShaderProgram;
  GLuint mVertShader;
  GLuint mFragShader;


  //-----------------------------------------------------------------------------
  // OpenGL related functions
  //-----------------------------------------------------------------------------

  /// Set up the camera.
  void SetupCamera()
  {
    mMesh.BoundingBox(mAABBMin, mAABBMax);
    mCameraLookAt = (mAABBMax + mAABBMin) * 0.5f;
    float delta = (mAABBMax - mAABBMin).Abs();
    mCameraPosition = Vector3(mCameraLookAt.x,
                             mCameraLookAt.y - 0.8f * delta,
                             mCameraLookAt.z + 0.2f * delta);
  }

  /// Initialize the GLSL shader (requires OpenGL 2.0 or better).
  void InitShader()
  {
    const GLchar * src[1];

    // Load vertex shader
    mVertShader = glCreateShader(GL_VERTEX_SHADER);
    src[0] = (const GLchar *) phongVertSrc;
    glShaderSource(mVertShader, 1, src, NULL);

    // Load fragment shader
    mFragShader = glCreateShader(GL_FRAGMENT_SHADER);	
    src[0] = (const GLchar *) phongFragSrc;
    glShaderSource(mFragShader, 1, src, NULL);

    int status;

    // Compile the vertex shader
    glCompileShader(mVertShader);
    glGetShaderiv(mVertShader, GL_COMPILE_STATUS, &status);
    if(!status)
      throw runtime_error("Could not compile vertex shader.");

    // Compile the fragment shader
    glCompileShader(mFragShader);
    glGetShaderiv(mFragShader, GL_COMPILE_STATUS, &status);
    if(!status)
      throw runtime_error("Could not compile fragment shader.");

    // Link the shader program
    mShaderProgram = glCreateProgram();
    glAttachShader(mShaderProgram, mVertShader);
    glAttachShader(mShaderProgram, mFragShader);
    glLinkProgram(mShaderProgram);
    glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
    if(!status)
      throw runtime_error("Could not link shader program.");

    mUseShader = true;
  }

  /// Initialize the texture.
  void InitTexture(const char * aFileName)
  {
    unsigned char * data = 0;
    int width = 256, height = 256, components = 1;

    // Load texture from a JPEG file
    if(aFileName)
    {
      FILE * inFile = fopen(aFileName, "rb");
      if((inFile == NULL) && (mFilePath.size() > 0))
      {
        // Try the same path as the mesh file
        string name = mFilePath + string(aFileName);
        inFile = fopen(name.c_str(), "rb");
      }
      if(inFile != NULL)
      {
        cout << "Loading texture (" << aFileName << ")..." << endl;
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, inFile);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);
        width = cinfo.output_width;
        height = cinfo.output_height;
        components = cinfo.output_components;
        data = new unsigned char[width * height * components];
        for(int i = 0; i < height; ++ i)
        {
          unsigned char * scanLines[1];
          scanLines[0] = &data[(height - 1 - i) * width * components];
          jpeg_read_scanlines(&cinfo, scanLines, 1);
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
      }
    }

    // If no texture was loaded
    if(!data)
    {
      cout << "Loading texture (dummy)..." << endl;

      // Create a default, synthetic texture
      width = height = 256;
      components = 1;
      data = new unsigned char[width * height * components];
      for(int y = 0; y < height; ++ y)
      {
        for(int x = 0; x < width; ++ x)
        {
          if(((x & 0x000f) == 0) || ((y & 0x000f) == 0))
            data[y * width + x] = 192;
          else
            data[y * width + x] = 255;
        }
      }
    }

    // Upload the texture to OpenGL
    if(data)
      glGenTextures(1, &mTexHandle);
    else
      mTexHandle = 0;
    if(mTexHandle)
    {
      // Determine the color format
      GLuint format;
      if(components == 3)
        format = GL_RGB;
      else if(components == 4)
        format = GL_RGBA;
      else
        format = GL_LUMINANCE;

      glBindTexture(GL_TEXTURE_2D, mTexHandle);

      if(GLEW_VERSION_1_4)
      {
        // Generate mipmaps automatically and use them
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      }
      else
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, (GLvoid *) data);
    }
  }

  /// Set up the scene lighting.
  void SetupLighting()
  {
    GLfloat pos[4], ambient[4], diffuse[4], specular[4];

    // Set scene lighting properties
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    ambient[0] = 0.08;
    ambient[1] = 0.08;
    ambient[2] = 0.08;
    ambient[3] = 1.0;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Set-up head light (GL_LIGHT0)
    pos[0] = mCameraPosition.x;
    pos[1] = mCameraPosition.y;
    pos[2] = mCameraPosition.z;
    pos[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    ambient[0] = 0.0f;
    ambient[1] = 0.0f;
    ambient[2] = 0.0f;
    ambient[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    diffuse[0] = 1.0f;
    diffuse[1] = 1.0f;
    diffuse[2] = 1.0f;
    diffuse[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    specular[0] = 1.0f;
    specular[1] = 1.0f;
    specular[2] = 1.0f;
    specular[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glEnable(GL_LIGHT0);
  }

  /// Set up the material.
  void SetupMaterial()
  {
    GLfloat specular[4], emission[4];

    // Set up the material
    specular[0] = 0.3f;
    specular[1] = 0.3f;
    specular[2] = 0.3f;
    specular[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    emission[0] = 0.0f;
    emission[1] = 0.0f;
    emission[2] = 0.0f;
    emission[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40.0f);

    // Use color material for the diffuse and ambient components
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
  }

  /// Draw a mesh
  void DrawMesh(Mesh &aMesh)
  {
    // We always have vertices
    glVertexPointer(3, GL_FLOAT, 0, &aMesh.mVertices[0]);
    glEnableClientState(GL_VERTEX_ARRAY);

    // Do we have normals?
    if(aMesh.mNormals.size() == aMesh.mVertices.size())
    {
      glNormalPointer(GL_FLOAT, 0, &aMesh.mNormals[0]);
      glEnableClientState(GL_NORMAL_ARRAY);
    }

    // Do we have texture coordinates?
    if(aMesh.mTexCoords.size() == aMesh.mVertices.size())
    {
      glTexCoordPointer(2, GL_FLOAT, 0, &aMesh.mTexCoords[0]);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    // Do we have colors?
    if(aMesh.mColors.size() == aMesh.mVertices.size())
    {
      glColorPointer(4, GL_FLOAT, 0, &aMesh.mColors[0]);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    // Use glDrawElements to draw the triangles...
    glShadeModel(GL_SMOOTH);
    glDrawElements(GL_TRIANGLES, aMesh.mIndices.size(), GL_UNSIGNED_INT,
                   &aMesh.mIndices[0]);

    // We do not use the client state anymore...
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
  }

  // Load a file to the mesh
  void LoadFile(const char * aFileName, const char * aOverrideTexture)
  {
    // Get the file size
    ifstream f(aFileName, ios::in | ios::binary);
    if(f.fail())
      throw runtime_error("Unable to open the file.");
    f.seekg(0, ios_base::end);
    long tmpFileSize = (long) f.tellg();
    f.close();

    // Load the mesh
    cout << "Loading " << aFileName << "..." << flush;
    int t = glutGet(GLUT_ELAPSED_TIME);
    ImportMesh(aFileName, mMesh);
    t = glutGet(GLUT_ELAPSED_TIME) - t;
    cout << "done (" << t << " ms)" << endl;

    // Get the file name (excluding the path), and the path (excluding the file name)
    mFileName = string(aFileName);
    size_t lastSlash = mFileName.rfind("/");
    if(lastSlash == string::npos)
      lastSlash = mFileName.rfind("\\");
    if(lastSlash != string::npos)
    {
      mFilePath = mFileName.substr(0, lastSlash + 1);
      mFileName = mFileName.substr(lastSlash + 1);
    }
    else
      mFilePath = string("");

    // The temporary file size is now the official file size...
    mFileSize = tmpFileSize;

    // Set window title
    string windowCaption = string("OpenCTM viewer - ") + mFileName;
    glutSetWindowTitle(windowCaption.c_str());

    // If the file did not contain any normals, calculate them now...
    if(mMesh.mNormals.size() != mMesh.mVertices.size())
    {
      cout << "Calculating normals..." << flush;
      int t = glutGet(GLUT_ELAPSED_TIME);
      mMesh.CalculateNormals();
      t = glutGet(GLUT_ELAPSED_TIME) - t;
      cout << "done (" << t << " ms)" << endl;
    }

    // Load the texture
    if(mTexHandle)
      glDeleteTextures(1, &mTexHandle);
    mTexHandle = 0;
    if(mMesh.mTexCoords.size() == mMesh.mVertices.size())
    {
      string texFileName = mMesh.mTexFileName;
      if(aOverrideTexture)
        texFileName = string(aOverrideTexture);
      if(texFileName.size() > 0)
        InitTexture(texFileName.c_str());
      else
        InitTexture(0);
    }

    // Setup texture parameters for the shader
    if(mUseShader)
    {
      glUseProgram(mShaderProgram);

      // Set the uUseTexture uniform
      GLint useTexLoc = glGetUniformLocation(mShaderProgram, "uUseTexture");
      if(useTexLoc >= 0)
        glUniform1i(useTexLoc, glIsTexture(mTexHandle));

      // Set the uTex uniform
      GLint texLoc = glGetUniformLocation(mShaderProgram, "uTex");
      if(texLoc >= 0)
        glUniform1i(texLoc, 0);

      glUseProgram(0);
    }

    // Load the mesh into a displaylist
    if(mDisplayList)
      glDeleteLists(mDisplayList, 1);
    mDisplayList = glGenLists(1);
    glNewList(mDisplayList, GL_COMPILE);
    DrawMesh(mMesh);
    glEndList();

    // Init the camera for the new mesh
    SetupCamera();
  }

  // Draw a string using GLUT. The string is shown on top of an alpha-blended
  // quad.
  void DrawString(string aString, int x, int y)
  {
    // Calculate the size of the string box
    int x0 = x, y0 = y;
    int x1 = x0, y1 = y0;
    int x2 = x0, y2 = y0;
    for(unsigned int i = 0; i < aString.size(); ++ i)
    {
      int c = (int) aString[i];
      if(c == (int) 10)
      {
        x2 = x;
        y2 += 13;
      }
      else if(c != (int) 13)
      {
        x2 += glutBitmapWidth(GLUT_BITMAP_8_BY_13, c);
        if(x2 > x1) x1 = x2;
      }
    }
    y1 = y2 + 13;

    // Draw a alpha blended box
    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2i(x0 - 4, y0 - 3);
    glVertex2i(x1 + 4, y0 - 3);
    glVertex2i(x1 + 4, y1 + 4);
    glVertex2i(x0 - 4, y1 + 4);
    glEnd();
    glDisable(GL_BLEND);
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x0 - 4, y0 - 3);
    glVertex2i(x1 + 4, y0 - 3);
    glVertex2i(x1 + 4, y1 + 4);
    glVertex2i(x0 - 4, y1 + 4);
    glEnd();

    // Print the text
    glColor3f(1.0f, 1.0f, 1.0f);
    x2 = x;
    y2 = y + 13;
    for(unsigned int i = 0; i < aString.size(); ++ i)
    {
      int c = (int) aString[i];
      if(c == (int) 10)
      {
        x2 = x;
        y2 += 13;
      }
      else if(c != (int) 13)
      {
        glRasterPos2i(x2, y2);
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
        x2 += glutBitmapWidth(GLUT_BITMAP_8_BY_13, c);
      }
    }
  }

  // Draw 2D overlay
  void Draw2DOverlay()
  {
    // Setup the matrices for a width x height 2D screen
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double) mWidth, (double) mHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Setup the rendering pipeline for 2D rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Render an info string
    stringstream s;
    s << mFileName << " (" << (mFileSize + 512) / 1024 << "KB)" << endl;
    s << mMesh.mVertices.size() << " vertices" << endl;
    s << mMesh.mIndices.size() / 3 << " triangles";
    DrawString(s.str(), 10, mHeight - 50);
  }


  //-----------------------------------------------------------------------------
  // Actions (user activated functions)
  //-----------------------------------------------------------------------------

  /// Open another file
  void ActionOpenFile()
  {
    SysOpenDialog od;
    od.mFilters.push_back(string("All supported 3D files|*.ctm;*.ply;*.stl;*.3ds;*.dae;*.obj"));
    od.mFilters.push_back(string("OpenCTM (.ctm)|*.ctm"));
    od.mFilters.push_back(string("Stanford triangle format (.ply)|*.ply"));
    od.mFilters.push_back(string("Stereolitography (.stl)|*.stl"));
    od.mFilters.push_back(string("3D Studio (.3ds)|*.3ds"));
    od.mFilters.push_back(string("COLLADA (.dae)|*.dae"));
    od.mFilters.push_back(string("Wavefront geometry file (.obj)|*.obj"));
    od.mFileName = mFileName;
    if(od.Show())
    {
      try
      {
        LoadFile(od.mFileName.c_str(), NULL);
        glutPostRedisplay();
      }
      catch(exception &e)
      {
        SysMessageBox mb;
        mb.mMessageType = SysMessageBox::mtError;
        mb.mCaption = "Error";
        mb.mText = string(e.what());
        mb.Show();
      }
    }
  }

  /// Save the file
  void ActionSaveFile()
  {
    SysSaveDialog sd;
    sd.mFilters.push_back(string("OpenCTM (.ctm)|*.ctm"));
    sd.mFilters.push_back(string("Stanford triangle format (.ply)|*.ply"));
    sd.mFilters.push_back(string("Stereolitography (.stl)|*.stl"));
    sd.mFilters.push_back(string("3D Studio (.3ds)|*.3ds"));
    sd.mFilters.push_back(string("COLLADA (.dae)|*.dae"));
    sd.mFilters.push_back(string("Wavefront geometry file (.obj)|*.obj"));
    sd.mFileName = mFileName;
    if(sd.Show())
    {
      try
      {
        Options opt;
        ExportMesh(sd.mFileName.c_str(), mMesh, opt);
      }
      catch(exception &e)
      {
        SysMessageBox mb;
        mb.mMessageType = SysMessageBox::mtError;
        mb.mCaption = "Error";
        mb.mText = string(e.what());
        mb.Show();
      }
    }
  }

  /// Toggle wire frame view on/off
  void ActionToggleWireframe()
  {
    if(mPolyMode == GL_LINE)
      mPolyMode = GL_FILL;
    else
      mPolyMode = GL_LINE;
    glutPostRedisplay();
  }

  /// Exit program
  void ActionExit()
  {
    // Note: In freeglut you can do glutLeaveMainLoop(), which is more graceful
    exit(0);
  }

  /// Show a help dialog
  void ActionHelp()
  {
    stringstream helpText;
    helpText << "ctmviewer - OpenCTM file viewer" << endl;
    helpText << "Copyright (c) 2009 Marcus Geelnard" << endl << endl;
    helpText << "Keyboard actions:" << endl;
    helpText << "  o - Open file" << endl;
    helpText << "  s - Save file" << endl;
    helpText << "  w - Toggle wire frame view on/off" << endl;
    helpText << "  ESC - Exit program" << endl << endl;
    helpText << "Mouse control:" << endl;
    helpText << "  Left button  - Rotate camera" << endl;
    helpText << "  Right button - Zoom camera";

    SysMessageBox mb;
    mb.mMessageType = SysMessageBox::mtInformation;
    mb.mCaption = "Help";
    mb.mText = helpText.str();
    mb.Show();
  }


public:

  //-----------------------------------------------------------------------------
  // GLUT callback functions
  //-----------------------------------------------------------------------------

  /// Redraw function.
  void WindowRedraw(void)
  {
    // Set the viewport to be the entire window
    glViewport(0, 0, mWidth, mHeight);

    // Clear the buffer(s)
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw a gradient background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.5f, 0.7f);
    glVertex3f(-1.0f, -1.0f, 0.5f);
    glColor3f(0.3f, 0.4f, 0.7f);
    glVertex3f(1.0f, -1.0f, 0.5f);
    glColor3f(0.1f, 0.1f, 0.2f);
    glVertex3f(1.0f, 1.0f, 0.5f);
    glColor3f(0.1f, 0.15f, 0.24f);
    glVertex3f(-1.0f, 1.0f, 0.5f);
    glEnd();

    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio;
    if(mHeight == 0)
      ratio = 1.0f;
    else
      ratio = (float) mWidth / (float) mHeight;
    float range = (mAABBMax - mAABBMin).Abs() +
                  (mCameraPosition - mCameraLookAt).Abs();
    if(range < 1e-20f)
      range = 1e-20f;
    gluPerspective(60.0f, ratio, 0.01f * range, range);

    // Set up the camera modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(mCameraPosition.x, mCameraPosition.y, mCameraPosition.z,
              mCameraLookAt.x, mCameraLookAt.y, mCameraLookAt.z,
              0.0f, 0.0f, 1.0f);

    // Set up the lights
    SetupLighting();

    // Enable material shader
    if(mUseShader)
      glUseProgram(mShaderProgram);
    else
      glEnable(GL_LIGHTING);

    // Draw the mesh
    SetupMaterial();
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, mPolyMode);
    if(mTexHandle)
    {
      glBindTexture(GL_TEXTURE_2D, mTexHandle);
      glEnable(GL_TEXTURE_2D);
      glColor3f(1.0f, 1.0f, 1.0f);
    }
    else
      glColor3f(0.9f, 0.86f, 0.7f);
    glCallList(mDisplayList);
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Disable material shader
    if(mUseShader)
      glUseProgram(0);
    else
      glDisable(GL_LIGHTING);

    // Draw 2D overlay (information text etc)
    Draw2DOverlay();

    // Swap buffers
    glutSwapBuffers();
  }

  /// Resize function.
  void WindowResize(int w, int h)
  {
    // Store the new window size
    mWidth = w;
    mHeight = h;
  }

  /// Mouse click function
  void MouseClick(int button, int state, int x, int y)
  {
    if(button == GLUT_LEFT_BUTTON)
    {
      if(state == GLUT_DOWN)
        mMouseRotate = true;
      else if(state == GLUT_UP)
        mMouseRotate = false;
    }
    else if(button == GLUT_RIGHT_BUTTON)
    {
      if(state == GLUT_DOWN)
        mMouseZoom = true;
      else if(state == GLUT_UP)
        mMouseZoom = false;
    }
    mOldMouseX = x;
    mOldMouseY = y;
  }

  /// Mouse move function
  void MouseMove(int x, int y)
  {
    float deltaX = (float) x - (float) mOldMouseX;
    float deltaY = (float) y - (float) mOldMouseY;
    mOldMouseX = x;
    mOldMouseY = y;

    if(mMouseRotate)
    {
      // Calculate delta angles
      float scale = 3.0f;
      if(mHeight > 0)
        scale /= (float) mHeight;
      float deltaTheta = -scale * deltaX;
      float deltaPhi = -scale * deltaY;

      // Adjust camera angles
      Vector3 viewVector = mCameraPosition - mCameraLookAt;
      float r = sqrtf(viewVector.x * viewVector.x +
                      viewVector.y * viewVector.y +
                      viewVector.z * viewVector.z);
      float phi, theta;
      if(r > 1e-20f)
      {
        phi = acosf(viewVector.z / r);
        theta = atan2f(viewVector.y, viewVector.x);
      }
      else
      {
        phi = viewVector.z > 0.0f ? 0.05f * PI : 0.95f * PI;
        theta = 0.0f;
      }
      phi += deltaPhi;
      theta += deltaTheta;
      if(phi > (0.95f * PI))
        phi = 0.95f * PI;
      else if(phi < (0.05f * PI))
        phi = 0.05f * PI;

      // Update the camera position
      viewVector.x = r * cos(theta) * sin(phi);
      viewVector.y = r * sin(theta) * sin(phi);
      viewVector.z = r * cos(phi);
      mCameraPosition = mCameraLookAt + viewVector;

      glutPostRedisplay();
    }
    else if(mMouseZoom)
    {
      // Calculate delta angles
      float scale = 3.0f;
      if(mHeight > 0)
        scale /= (float) mHeight;
      float zoom = scale * deltaY;

      // Adjust camera zoom
      Vector3 viewVector = mCameraPosition - mCameraLookAt;
      viewVector = viewVector * powf(2.0f, zoom);

      // Update the camera position
      mCameraPosition = mCameraLookAt + viewVector;

      glutPostRedisplay();
    }
  }

  /// Keyboard function
  void KeyDown(unsigned char key, int x, int y)
  {
    if(key == 'o')
      ActionOpenFile();
    else if(key == 's')
      ActionSaveFile();
    else if(key == 'w')
      ActionToggleWireframe();
    else if(key == 27)
      ActionExit();
  }

  /// Keyboard function (special keys)
  void SpecialKeyDown(int key, int x, int y)
  {
    if(key == GLUT_KEY_F1)
      ActionHelp();
  }


  //-----------------------------------------------------------------------------
  // Application main code
  //-----------------------------------------------------------------------------

  /// Run the application
  void Run(int argc, char **argv)
  {
    try
    {
      // Clear internal state
      mFileName = "";
      mFilePath = "";
      mFileSize = 0;
      mWidth = 1;
      mHeight = 1;
      mOldMouseX = 0;
      mOldMouseY = 0;
      mMouseRotate = false;
      mMouseZoom = false;
      mDisplayList = 0;
      mPolyMode = GL_FILL;
      mTexHandle = 0;
      mUseShader = false;
      mShaderProgram = 0;
      mVertShader = 0;
      mFragShader = 0;

      // Init GLUT
      glutInit(&argc, argv);

      // Create the glut window
      glutInitWindowSize(640, 480);
      glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
      glutCreateWindow("OpenCTM viewer");

      // Init GLEW (for OpenGL 2.x support)
      if(glewInit() != GLEW_OK)
        throw runtime_error("Unable to initialize GLEW.");

      // Load the phong shader, if we can
      if(GLEW_VERSION_2_0)
        InitShader();
      else if(GLEW_VERSION_1_2)
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

      // Load the file
      const char * overrideTexName = NULL;
      if(argc >= 3)
        overrideTexName = argv[2];
      LoadFile(argv[1], overrideTexName);

      // Set the GLUT callback functions (these are bridged to the corresponding
      // class methods)
      glutReshapeFunc(GLUTWindowResize);
      glutDisplayFunc(GLUTWindowRedraw);
      glutMouseFunc(GLUTMouseClick);
      glutMotionFunc(GLUTMouseMove);
      glutKeyboardFunc(GLUTKeyDown);
      glutSpecialFunc(GLUTSpecialKeyDown);

      // Enter the main loop
      glutMainLoop();
    }
    catch(ctm_error &e)
    {
      SysMessageBox mb;
      mb.mMessageType = SysMessageBox::mtError;
      mb.mCaption = "Error";
      mb.mText = string("OpenCTM error: ") + string(e.what());
      mb.Show();
    }
    catch(exception &e)
    {
      SysMessageBox mb;
      mb.mMessageType = SysMessageBox::mtError;
      mb.mCaption = "Error";
      mb.mText = string(e.what());
      mb.Show();
    }
    cout << endl;
  }
};


//-----------------------------------------------------------------------------
// Bridge GLUT callback functions to class methods
//-----------------------------------------------------------------------------

// NOTE: This is just a hack to be able to reference the application class
// object from the GLUT callback functions, since there is no way (afaik) to
// pass user data (i.e. the object reference) through GLUT...
static GLViewer * gGLViewer = NULL;

/// Redraw function.
void GLUTWindowRedraw(void)
{
  if(gGLViewer)
    gGLViewer->WindowRedraw();
}

/// Resize function.
void GLUTWindowResize(int w, int h)
{
  if(gGLViewer)
    gGLViewer->WindowResize(w, h);
}

/// Mouse click function
void GLUTMouseClick(int button, int state, int x, int y)
{
  if(gGLViewer)
    gGLViewer->MouseClick(button, state, x, y);
}

/// Mouse move function
void GLUTMouseMove(int x, int y)
{
  if(gGLViewer)
    gGLViewer->MouseMove(x, y);
}

/// Keyboard function
void GLUTKeyDown(unsigned char key, int x, int y)
{
  if(gGLViewer)
    gGLViewer->KeyDown(key, x, y);
}

/// Keyboard function (special keys)
void GLUTSpecialKeyDown(int key, int x, int y)
{
  if(gGLViewer)
    gGLViewer->SpecialKeyDown(key, x, y);
}


//-----------------------------------------------------------------------------
// Program startup
//-----------------------------------------------------------------------------

/// Program entry.
int main(int argc, char **argv)
{
  // Was the program invoked correctly?
  if((argc < 2) || (argc > 3))
  {
    // ...usage
    stringstream s;
    s << "ctmviewer file [texturefile]" << endl;

    // ...supported formats
    s << endl << "Supported file formats:" << endl << endl;
    list<string> formatList;
    SupportedFormats(formatList);
    for(list<string>::iterator i = formatList.begin(); i != formatList.end(); ++ i)
      s << "  " << (*i) << endl;

    // Show the message
    SysMessageBox mb;
    mb.mCaption = "Usage";
    mb.mText = s.str();
    mb.Show();

    return 0;
  }

  // Run the application class
  gGLViewer = new GLViewer;
  gGLViewer->Run(argc, argv);
  delete gGLViewer;
  gGLViewer = NULL;

  return 0;
}
