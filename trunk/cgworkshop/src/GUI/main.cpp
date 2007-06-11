#include "GUI.h"

#include <stdio.h>

#include <GL/glut.h>

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void myRenderFunc()
{
	CGUI::GetInstance()->Render();
}

///////////////////////////////////////////////////////////////////////////////////

void myReshapeFunc(int x , int y )
{
	CGUI::GetInstance()->Reshape(x, y);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////////

void myKeysFunc( unsigned char key, int x, int y )
{
	CGUI::GetInstance()->KeysAction(key, x, y);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////////

void myMouseFunc(int button, int state, int x, int y)
{
	CGUI::GetInstance()->MouseAction(button, state, x, y);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////////

void myMotionFunc(int x, int y)
{
	CGUI::GetInstance()->MouseMove(x,y);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////////

void myInitOpenGL()
{
	glDisable(GL_FOG); 
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	glPolygonMode(GL_FRONT, GL_FILL);

	glEnable( GL_TEXTURE_2D );

	glLineWidth(3);
	glPointSize(5);
	glClearColor(1.0, 1.0f, 1.0f, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv)
{
	if ( argc != 2 )
	{
		printf("Usage: image_path \n");
		return -1;
	}

	CGUI::GetInstance()->Setup(argv[1]);

	// specify parameters for the window
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( CGUI::GetInstance()->GetWidth(), CGUI::GetInstance()->GetHeight() );
	
	// create the window
	glutCreateWindow( "Let's scribble" );
	
	// assign the function to the events
	glutDisplayFunc ( myRenderFunc  );
	glutReshapeFunc ( myReshapeFunc );
	glutKeyboardFunc( myKeysFunc	);
	glutMouseFunc	( myMouseFunc	);
	glutMotionFunc	( myMotionFunc	);
	glutIdleFunc	( NULL			);
	
	// initialize the OpenGL variables
	myInitOpenGL();
	
	CGUI::GetInstance()->LoadTextures();

	// start the main loop
	glutMainLoop();

	return 0;
}