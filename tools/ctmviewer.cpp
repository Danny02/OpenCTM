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
#include "mesh.h"
#include "ctm.h"
#include "ply.h"
#include "stl.h"
#include "3ds.h"

using namespace std;


// We need PI
#ifndef PI
#define PI 3.141592653589793238462643f
#endif

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



// Global variables (this is a simple program, after all)
string fileName("");
long fileSize = 0;

int width = 1, height = 1;
int oldMouseX = 0, oldMouseY = 0;
bool mouseRotate = false;
bool mouseZoom = false;

Mesh mesh;
Vector3 aabbMin, aabbMax;
Vector3 cameraPosition, cameraLookAt;
GLuint displayList = 0;
GLenum polyMode = GL_FILL;

GLuint texHandle = 0;

bool useShader = false;
GLuint shaderProgram = 0;
GLuint vertShader = 0;
GLuint fragShader = 0;

/// Set up the scene.
void SetupScene()
{
  mesh.BoundingBox(aabbMin, aabbMax);
  cameraLookAt = (aabbMax + aabbMin) * 0.5f;
  float delta = (aabbMax - aabbMin).Abs();
  cameraPosition = Vector3(cameraLookAt.x,
                           cameraLookAt.y - 0.8f * delta,
                           cameraLookAt.z + 0.2f * delta);
}

/// Load a text file as a string. Use C++ delete to free the memory.
char * LoadTextFile(const char * aFileName)
{
  ifstream f(aFileName, ios::binary | ios::in);
  if(f.fail())
    return NULL;

  f.seekg(0, ios_base::end);
  unsigned int length = f.tellg();
  f.seekg(0, ios_base::beg);

  char * result = (char *) new char[length + 1];
  if(result)
  {
    f.read(result, length);
    result[length] = 0;
  }

  return result;
}

/// Initialize the GLSL shader (requires OpenGL 2.0 or better).
void InitShader()
{
  // Load vertex shader
  char *vs = LoadTextFile("phong.vert");
  if(!vs)
    throw runtime_error("Unable to load vertex shader.");
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, (const GLchar **) &vs, NULL);
  delete vs;

  // Load fragment shader
  char *fs = LoadTextFile("phong.frag");
  if(!fs)
    throw runtime_error("Unable to load fragment shader.");
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);	
  glShaderSource(fragShader, 1, (const GLchar **) &fs, NULL);
  delete fs;

  int status;

  // Compile the vertex shader
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
  if(!status)
    throw runtime_error("Could not compile vertex shader.");

  // Compile the fragment shader
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
  if(!status)
    throw runtime_error("Could not compile fragment shader.");

  // Link the shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertShader);
  glAttachShader(shaderProgram, fragShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
  if(!status)
    throw runtime_error("Could not link shader program.");

  glUseProgram(shaderProgram);

  // Set the uUseTexture uniform
  GLint useTexLoc = glGetUniformLocation(shaderProgram, "uUseTexture");
  if(useTexLoc >= 0)
    glUniform1i(useTexLoc, glIsTexture(texHandle));

  // Set the uTex uniform
  GLint texLoc = glGetUniformLocation(shaderProgram, "uTex");
  if(texLoc >= 0)
    glUniform1i(texLoc, 0);

  glUseProgram(0);

  useShader = true;
}

/// Initialize the texture.
void InitTexture(const char * aFileName)
{
  unsigned char * data = 0;
  int width = 256, height = 256, components = 1;

  // Load texture from a JPEG file
  if(aFileName)
  {
    FILE * inFile;
    if((inFile = fopen(aFileName, "rb")) != NULL)
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
    glGenTextures(1, &texHandle);
  else
    texHandle = 0;
  if(texHandle)
  {
    // Determine the color format
    GLuint format;
    if(components == 3)
      format = GL_RGB;
    else if(components == 4)
      format = GL_RGBA;
    else
      format = GL_LUMINANCE;

    glBindTexture(GL_TEXTURE_2D, texHandle);

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
  pos[0] = cameraPosition.x;
  pos[1] = cameraPosition.y;
  pos[2] = cameraPosition.z;
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

// Draw a string using GLUT. The string is shown on top of an alpha-blended
// quad.
void DrawString(string &aString, int x, int y)
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

// Draw some textual information to the screen
void DrawInfoText()
{
  // Setup the matrices for a width x height 2D screen
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (double) width, (double) height, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Setup the rendering pipeline for text rendering
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Create a message string
  stringstream s;
  s << fileName << " (" << (fileSize + 512) / 1024 << "KB)" << endl;
  s << mesh.mVertices.size() << " vertices" << endl;
  s << mesh.mIndices.size() / 3 << " triangles";

  // Render the string
  string msg = s.str();
  DrawString(msg, 7, 6);
}

/// Redraw function.
void WindowRedraw(void)
{
  // Set the viewport to be the entire window
  glViewport(0, 0, width, height);

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
  if(height == 0)
    ratio = 1.0f;
  else
    ratio = (float) width / (float) height;
  float range = (aabbMax - aabbMin).Abs() +
                (cameraPosition - cameraLookAt).Abs();
  if(range < 1e-20f)
    range = 1e-20f;
  gluPerspective(60.0f, ratio, 0.01f * range, range);

  // Set up the camera modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
            cameraLookAt.x, cameraLookAt.y, cameraLookAt.z,
            0.0f, 0.0f, 1.0f);

  // Set up the lights
  SetupLighting();

  // Enable material shader
  if(useShader)
    glUseProgram(shaderProgram);
  else
    glEnable(GL_LIGHTING);

  // Draw the mesh
  SetupMaterial();
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, polyMode);
  if(texHandle)
  {
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
  }
  else
    glColor3f(0.9f, 0.86f, 0.7f);
  glCallList(displayList);
  glDisable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Disable material shader
  if(useShader)
    glUseProgram(0);
  else
    glDisable(GL_LIGHTING);

  // Draw information text
  DrawInfoText();

  // Swap buffers
  glutSwapBuffers();
}

/// Resize function.
void WindowResize(int w, int h)
{
  // Store the new window size
  width = w;
  height = h;
}

/// Mouse click function
static void MouseClick(int button, int state, int x, int y)
{
  if(button == GLUT_LEFT_BUTTON)
  {
    if(state == GLUT_DOWN)
      mouseRotate = true;
    else if(state == GLUT_UP)
      mouseRotate = false;
  }
  else if(button == GLUT_RIGHT_BUTTON)
  {
    if(state == GLUT_DOWN)
      mouseZoom = true;
    else if(state == GLUT_UP)
      mouseZoom = false;
  }
  oldMouseX = x;
  oldMouseY = y;
}

/// Mouse move function
static void MouseMove(int x, int y)
{
  float deltaX = (float) x - (float) oldMouseX;
  float deltaY = (float) y - (float) oldMouseY;
  oldMouseX = x;
  oldMouseY = y;

  if(mouseRotate)
  {
    // Calculate delta angles
    float scale = 3.0f;
    if(height > 0)
      scale /= (float) height;
    float deltaTheta = -scale * deltaX;
    float deltaPhi = -scale * deltaY;

    // Adjust camera angles
    Vector3 viewVector = cameraPosition - cameraLookAt;
    float r = sqrtf(viewVector.x * viewVector.x + viewVector.y * viewVector.y + viewVector.z * viewVector.z);
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
    cameraPosition = cameraLookAt + viewVector;

    glutPostRedisplay();
  }
  else if(mouseZoom)
  {
    // Calculate delta angles
    float scale = 3.0f;
    if(height > 0)
      scale /= (float) height;
    float zoom = scale * deltaY;

    // Adjust camera zoom
    Vector3 viewVector = cameraPosition - cameraLookAt;
    viewVector = viewVector * powf(2.0f, zoom);

    // Update the camera position
    cameraPosition = cameraLookAt + viewVector;

    glutPostRedisplay();
  }
}

/// Keyboard function
void KeyDown(unsigned char key, int x, int y)
{
  if(key == 27)
    // Note: In freeglut you can do glutLeaveMainLoop(), which is more graceful
    exit(0);

  if(key == 'w')
  {
    if(polyMode == GL_LINE)
      polyMode = GL_FILL;
    else
      polyMode = GL_LINE;
    glutPostRedisplay();
  }
}

/// Program entry.
int main(int argc, char **argv)
{
  // Usage?
  if((argc < 2) || (argc > 3))
  {
    cout << "Usage: ctmviewer file [texturefile]" << endl;
    return 0;
  }

  try
  {
    // Get the file name and size
    fileName = string(argv[1]);
    ifstream f(fileName.c_str(), ios::in | ios::binary);
    if(f.fail())
      throw runtime_error("Unable to open the file.");
    f.seekg(0, ios_base::end);
    fileSize = (long) f.tellg();
    f.close();

    // Init GLUT
    glutInit(&argc, argv);

    // Load the file
    cout << "Loading " << argv[1] << "..." << flush;
    int t = glutGet(GLUT_ELAPSED_TIME);
    string fileExt = UpperCase(ExtractFileExt(string(argv[1])));
    if(fileExt == string(".PLY"))
      Import_PLY(argv[1], mesh);
    else if(fileExt == string(".STL"))
      Import_STL(argv[1], mesh);
    else if(fileExt == string(".3DS"))
      Import_3DS(argv[1], mesh);
    else if(fileExt == string(".CTM"))
      Import_CTM(argv[1], mesh);
    else
      throw runtime_error("Unknown input file extension.");
    t = glutGet(GLUT_ELAPSED_TIME) - t;
    cout << "done (" << t << " ms)" << endl;

    // If the file did not contain any normals, calculate them now...
    if(mesh.mNormals.size() != mesh.mVertices.size())
    {
      cout << "Calculating normals..." << flush;
      int t = glutGet(GLUT_ELAPSED_TIME);
      mesh.CalculateNormals();
      t = glutGet(GLUT_ELAPSED_TIME) - t;
      cout << "done (" << t << " ms)" << endl;
    }

    // Init scene
    SetupScene();

    // Create the glut window
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    string windowCaption = string("OpenCTM viewer - ") + string(argv[1]);
    glutCreateWindow(windowCaption.c_str());
    glutReshapeFunc(WindowResize);
    glutDisplayFunc(WindowRedraw);
    glutMouseFunc(MouseClick);
    glutMotionFunc(MouseMove);
    glutKeyboardFunc(KeyDown); 

    // Init GLEW (for OpenGL 2.x support)
    if(glewInit() != GLEW_OK)
      throw runtime_error("Unable to initialize GLEW.");

    // Load the texture
    if(mesh.mTexCoords.size() == mesh.mVertices.size())
    {
      string texFileName = mesh.mTexFileName;
      if(argc >= 3)
        texFileName = string(argv[2]);
      if(texFileName.size() > 0)
        InitTexture(texFileName.c_str());
      else
        InitTexture(0);
    }

    // Load the phong shader, if we can
    if(GLEW_VERSION_2_0)
      InitShader();
    else if(GLEW_VERSION_1_2)
      glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    // Load the mesh into a displaylist
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    DrawMesh(mesh);
    glEndList();

    // Enter the main loop
    glutMainLoop();
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }
}
