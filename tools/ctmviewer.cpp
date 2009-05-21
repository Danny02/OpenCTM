#include <GL/glut.h>
#include <iostream>
#include "mesh.h"

using namespace std;

// Global variables (this is a simple program, after all)
Mesh mesh;
int width = 1, height = 1;
Vector3 cameraPosition;

/// Set up the scene.
void SetupScene()
{
  cameraPosition = Vector3(0.0f, -10.0f, 2.0f);
}

/// Set up the scene lighting.
void SetupLighting()
{
  GLfloat pos[4], ambient[4], diffuse[4], specular[4];

  // Set scene lighting properties
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  ambient[0] = 0.2;
  ambient[1] = 0.2;
  ambient[2] = 0.2;
  ambient[3] = 1.0;
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  // Set-up head light (GL_LIGHT0)
  pos[0] = cameraPosition.x;
  pos[1] = cameraPosition.y;
  pos[2] = cameraPosition.z;
  pos[3] = 1.0;
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  ambient[0] = 0.0;
  ambient[1] = 0.0;
  ambient[2] = 0.0;
  ambient[3] = 1.0;
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  diffuse[0] = 1.0;
  diffuse[1] = 1.0;
  diffuse[2] = 1.0;
  diffuse[3] = 1.0;
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  specular[0] = 1.0;
  specular[1] = 1.0;
  specular[2] = 1.0;
  specular[3] = 1.0;
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glEnable(GL_LIGHT0);
}

/// Redraw function.
void WindowRedraw(void)
{
  // Clear the buffer(s)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up perspective projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float ratio;
  if(height == 0)
    ratio = 1.0f;
  else
    ratio = width / height;
  gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

  // Set up the camera modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f);

  // Set up the lights
  SetupLighting();
  glEnable(GL_LIGHTING);

  // Draw the mesh
  glEnable(GL_DEPTH_TEST);
  glColor3f(1.0f, 1.0f, 1.0f);
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

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);
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

    // Create the glut window
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("OpenCTM viewer");
    glutReshapeFunc(WindowResize);
    glutDisplayFunc(WindowRedraw);

    // Init scene
    SetupScene();

    // Enter the main loop
    glutMainLoop();
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }
}
