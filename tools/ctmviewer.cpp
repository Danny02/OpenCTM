#include <GL/glut.h>
#include <iostream>
#include "mesh.h"

using namespace std;

Mesh mesh;
int width = 1, height = 1;

void WindowResize(int w, int h)
{
  // Store the new window size
  width = w;
  height = h;

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);
}

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
  gluLookAt(0.0f, -10.0f, 3.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f);

  // Draw the mesh
  glEnable(GL_DEPTH_TEST);
  glColor3f(1.0f, 1.0f, 1.0f);
  mesh.Draw();

  // Swap buffers
  glutSwapBuffers();
}


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
    // Load the file
    mesh.LoadFromFile(argv[1]);

    // Init GLUT
    glutInit(&argc, argv);
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
