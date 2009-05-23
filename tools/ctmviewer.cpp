#include <iostream>
#include "mesh.h"

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// Global variables (this is a simple program, after all)
Mesh mesh;
int width = 1, height = 1;
Vector3 cameraPosition, cameraLookAt;

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
  // Set the viewport to be the entire window
  glViewport(0, 0, width, height);

  // Clear the buffer(s)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
  glEnable(GL_DEPTH_TEST);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
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

    // Enter the main loop
    glutMainLoop();
  }
  catch(exception &e)
  {
    cout << "Error: " << e.what() << endl;
  }
}
