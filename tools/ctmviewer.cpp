#include <GL/glut.h>
#include <iostream>
#include "mesh.h"

using namespace std;

Mesh mesh;

void WindowResize(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	float ratio = 1.0f * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);
}

void WindowRedraw(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the camera
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
