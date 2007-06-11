
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

#include <highgui.h>

#include "GUI.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *image, unsigned int &id );

///////////////////////////////////////////////////////////////////////////////////

void CGUI::Render()
{
	// clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// initialize the modelview matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Make the texture the current one
	glBindTexture(GL_TEXTURE_2D, m_textures[0]);
	
	// render the left image
	glBegin(GL_QUADS);
	{
		glTexCoord2f( 0.0, 1.0 );
		glVertex3f( m_orthoBBox[0], m_orthoBBox[3], 0);
		glTexCoord2f( 0.0, 0.0 );
		glVertex3f( m_orthoBBox[0], m_orthoBBox[1], 0);
		glTexCoord2f( 1.0, 0.0 );
		glVertex3f( m_orthoBBox[2], m_orthoBBox[1], 0);
		glTexCoord2f( 1.0, 1.0 );
		glVertex3f( m_orthoBBox[2], m_orthoBBox[3], 0);
	}
	glEnd();

	// render the left vectors
	if ( !m_scribbles.empty() )
	{
		// as lines
		glBegin( GL_LINES );
		{

			for ( int i = 0; i < m_scribbles.size(); i ++)
				m_scribbles[i].Draw();
		}

		glEnd();

		// as points
		glBegin( GL_POINTS);
		{
			for ( int i = 0; i < m_scribbles.size(); i ++)
				m_scribbles[i].Draw();
		}
		glEnd();

	}

	// reset the color
	glColor3f( 1,1,1 );

	// swap buffers
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::Reshape(int x , int y)
{
	// don't allow the window to be resized
	if (x != m_nWindowWidth || y != m_nWindowHeight) 
	{
		glutReshapeWindow(m_nWindowWidth, m_nWindowHeight);
		return;
	}

	SetWindowSize(x,y);

	// define the viewport
	glViewport( 0, 0, x, y );
	
	// initialize the projection matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	float ratio = (float)x / (float)y;

	// the orthogonal bounding box
	m_orthoBBox[0] = -(float)x * 0.5 * ratio;		// MIN X
	m_orthoBBox[1] = -(float)y * 0.5;				// MIN Y
	m_orthoBBox[2] =  (float)x * 0.5 * ratio;		// MAX X
	m_orthoBBox[3] =  (float)y * 0.5;				// MAX Y

	// the orthogonal projection
	glOrtho( m_orthoBBox[0],	// left
			 m_orthoBBox[2],	// right
			 m_orthoBBox[1],	// bottom
			 m_orthoBBox[3],	// top
				 1, -1	);	// near & far
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::KeysAction( unsigned char key, int x, int y )
{
	switch( key )
	{

	case 'q':
		exit(0);
		break;
		
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::MouseAction(int button, int state, int x, int y)
{
	// find the case of the key
	switch ( button )
	{
	case GLUT_LEFT_BUTTON:
		
		// released the left button
		if ( state == GLUT_UP )
		{
			int height = m_pImg->height;
			int width  = m_pImg->width;

			float ratio_x = (float)2 * width / GetWidth();
			float ratio_y = (float)	  height / GetHeight();

			leftVecs.push_back( (int)(ratio_x * x) );
			leftVecs.push_back( (int)((GetHeight() - y) * ratio_y) );

			// set the end point of the vector
			m_scribbles.back().m_endPoint.SetPoint
			//leftVecsRnd.back().SetPoint
			( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
										 m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() ));

		}
			
			// pressed the left button
		if ( state == GLUT_DOWN )
		{
			int height = m_pImg->height;
			int width  = m_pImg->width;

			float ratio_x = (float)2  * width / GetWidth();
			float ratio_y = (float)	  height / GetHeight();

			leftVecs.push_back( (int)(ratio_x * x) );
			leftVecs.push_back( (int)((GetHeight() - y) * ratio_y) );

			// the start of the vector
			m_scribbles.push_back( CScribble ( CPoint( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
											m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() )),
											CPoint( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
											m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() )) ) );
											
				/*							
			leftVecsRnd.push_back( CPoint( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
											m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() )) );

			// second point is inserted for vector motion rendering
			leftVecsRnd.push_back( CPoint( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
											m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() )) );*/
		}
		break;
	default:
		break;
	}

}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::MouseMove(int x, int y)
{
	// set the end point of the vector
	m_scribbles.back().m_endPoint.SetPoint
	//leftVecsRnd.back().SetPoint
	( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
								 m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() ));
}

///////////////////////////////////////////////////////////////////////////////////

int CGUI::Setup(char * pszImagePath)
{
	// Load the input image
	m_pImg = cvLoadImage(pszImagePath,1);
	if (m_pImg == NULL)
		return -1;

	//if the image is upside down
	if( m_pImg->origin == IPL_ORIGIN_TL)
		cvFlip(m_pImg,m_pImg, 0);
	
	SetWindowSize(m_pImg->width, m_pImg->height);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::LoadTextures()
{
	loadTexture( GetImage(),  m_textures[0] );
}

///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *image, unsigned int &id )
{
	glGenTextures(1, &id);  
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D,image->nChannels,image->width,image->height,GL_BGR_EXT,GL_UNSIGNED_BYTE,image->imageData);  
}

///////////////////////////////////////////////////////////////////////////////////
