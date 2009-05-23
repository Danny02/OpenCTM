#include <iostream>
#include "mesh.h"

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;


// We need PI
#ifndef PI
#define PI 3.141592653589793238462643f
#endif


// Global variables (this is a simple program, after all)
Mesh mesh;
int width = 1, height = 1;
Vector3 cameraPosition, cameraLookAt;
int oldMouseX = 0, oldMouseY = 0;
bool mouseRotate = false;
bool mouseZoom = false;

/// Set up the scene.
void SetupScene()
{
  Vector3 min, max;
  mesh.BoundingBox(min, max);
  cameraLookAt = (max + min) * 0.5f;
  float delta = (max - min).Abs();
  cameraPosition = Vector3(cameraLookAt.x,
                           cameraLookAt.y - 1.5f * delta,
                           cameraLookAt.z + 0.4f * delta);
}

/// Set up the scene lighting.
void SetupLighting()
{
  GLfloat pos[4], ambient[4], diffuse[4], specular[4];

  // Set scene lighting properties
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  ambient[0] = 0.05;
  ambient[1] = 0.05;
  ambient[2] = 0.05;
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
  glColor3f(0.2f, 0.2f, 0.4f);
  glVertex3f(-1.0f, -1.0f, 0.5f);
  glVertex3f(1.0f, -1.0f, 0.5f);
  glColor3f(0.1f, 0.1f, 0.2f);
  glVertex3f(1.0f, 1.0f, 0.5f);
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
  gluPerspective(45.0f, ratio, 0.1f, 1000.0f);

  // Set up the camera modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
            cameraLookAt.x, cameraLookAt.y, cameraLookAt.z,
            0.0f, 0.0f, 1.0f);

  // Set up the lights
  SetupLighting();
  glEnable(GL_LIGHTING);

  // Draw the mesh
  SetupMaterial();
  glEnable(GL_DEPTH_TEST);
  glColor3f(0.9f, 0.86f, 0.7f);
  mesh.Draw();

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
    float scale = 2.0f;
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

/// Program entry.
int main(int argc, char **argv)
{
  // Usage?
  if(argc != 2)
  {
    cout << "Usage: ctmviewer file" << endl;
    return 0;
  }

  try
  {
    // Init GLUT
    glutInit(&argc, argv);

    // Load the file
    cout << "Loading " << argv[1] << "..." << flush;
    int t = glutGet(GLUT_ELAPSED_TIME);
    mesh.LoadFromFile(argv[1]);
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
    glutCreateWindow("OpenCTM viewer");
    glutReshapeFunc(WindowResize);
    glutDisplayFunc(WindowRedraw);
    glutMouseFunc(MouseClick);
    glutMotionFunc(MouseMove);

    // Enter the main loop
    glutMainLoop();
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }
}
