
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

#include <highgui.h>

#include "GUI.h"

#include "../fe/FeatureExtraction.h"
#include "../GMM/GMM.h"
#include "../GraphHandler.h"
#include "../Segmentator.h"


static const CvScalar s_colors[] = {{{0,0,1.0f}},{{0,1.0f,0}},{{0,1.0f,1.0f}},{{1.0f,1.0f,0}},{{0,0,1.0f}},{{0.5f,0.5f,1.0f}}
	,{{0.5f,0.5f,0.5f}},{{0.5f,1.0f,0.5f}},{{1.0f,0.5f,1.0f}},{{1.0f,0.2f,0.7f}}};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *image, unsigned int &id );

///////////////////////////////////////////////////////////////////////////////////

void CGUI::Render()
{
	CvScalar * pColor;

	// clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// initialize the modelview matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Make the texture the current one
	glBindTexture(GL_TEXTURE_2D, m_textures[0]);
	
	// render the image
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

	glDisable( GL_TEXTURE_2D );

	glBegin(GL_POINTS);
	{
		for ( int i = 0; i < m_scribbles.size(); i ++)
		{
			pColor = m_scribbles[i].GetColor();
			glColor3f(pColor->val[0],pColor->val[1],pColor->val[2]);
			m_scribbles[i].Draw();
		}
	}
	glEnd();

	glEnable( GL_TEXTURE_2D );

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

	case 'l':
		m_loader.Load(m_scribbles);
		break;

	case 's':
		m_loader.Save(m_scribbles);
		break;

	case 'c':
		m_scribbles.clear();
		break;

	case 'r':
		{

			int nScribbles = 0;
		CFeatureExtraction * fe;

		fe = new CFeatureExtraction(m_pImg);
		fe->Run();


		for (int i = 0; i < SCRIBBLE_NUMBER ; i++)
			if (m_scribbles[i].IsValid())
				nScribbles++;

		printf("nScribbles=%d\n", nScribbles);

		Segmentator seg(m_pImg, fe, m_scribbles, nScribbles);

		//GraphHandler::init_graph(m_pImg->height, m_pImg->width, fe->GetColorChannels());

 		seg.Segment();

		// display
		//IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
		//cvConvertScale(seg.getSegmentation(),outImg,255,0); 	
		
		char title[50];
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		//cvShowImage( title, outImg );
		
		for (int i=0;i<nScribbles;i++)
		{
			cvNamedWindow( title, 1 );
			cvShowImage( title, seg.GetSegmentedImage(i) );
			cvWaitKey(0);
			cvDestroyWindow(title);	
		}
		//cvSaveImage("test.bmp",outImg);
		//cvReleaseImage(&outImg);

		delete fe;

		}
		break;		
	default:
		break;
	}

	if (key >= '0' && key <= '9')
		m_nCurScribble = key - 48;
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::MouseAction(int button, int state, int x, int y)
{
	switch ( button )
	{
	case GLUT_LEFT_BUTTON:
		
		// released the left button
		if ( state == GLUT_UP )
			m_fScribbling = false;
			
			// pressed the left button
		if ( state == GLUT_DOWN )
		{
			if (m_nCurScribble < SCRIBBLE_NUMBER && m_nCurScribble >= 0)
			{	
				AddScribblePoints(x,y);
				
				m_fScribbling = true;
			}
		}

		break;

	default:
		break;
	}

}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::MouseMove(int x, int y)
{
	if (m_fScribbling)
		AddScribblePoints(x,y);	
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::AddScribblePoints(int x, int y)
{
	int origY;

	int height = m_pImg->height;
	int width  = m_pImg->width;

	float ratio_x = (float) width / GetWidth();
	float ratio_y = (float)	height / GetHeight();

	for (int i = x - 1; i <= x + 1; i++)
	{
		for (int j = y - 1; j <= y + 1; j++)
		{
			if( m_pImg->origin == IPL_ORIGIN_TL)
				origY = j;
			else
				origY = GetHeight() - j;

			CPointInt pI = CPointInt( (int)(ratio_x * i), (int)(origY * ratio_y));

			// the start of the vector
			CPointFloat pF = CPointFloat( m_orthoBBox[0] * ( 1 - 2 * (float)i / GetWidth() ),
										m_orthoBBox[3] * ( 1 - 2 * (float)j / GetHeight() ));

			m_scribbles[m_nCurScribble].AddImagePoint (pI);
			m_scribbles[m_nCurScribble].AddObjectPoint(pF);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////

int CGUI::Setup(char * pszImagePath, char *pScribbleFile /* = NULL */)
{
	// Load the input image
	m_pImg = cvLoadImage(pszImagePath,1);
	if (m_pImg == NULL)
		return -1;
	
	SetWindowSize(m_pImg->width, m_pImg->height);

	if (pScribbleFile)
		m_loader.Setup(pScribbleFile);

	m_nCurScribble = 0;
	m_scribbles.resize(SCRIBBLE_NUMBER);
	for (int i = 0; i < SCRIBBLE_NUMBER; i++)
	{
		m_scribbles[i].SetID(i);
		m_scribbles[i].SetColor(&s_colors[i]);
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::LoadTextures()
{
	loadTexture(GetImage(), m_textures[0]);
}

///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *pImage, unsigned int &id )
{
	//if the image is upside down
	if( pImage->origin == IPL_ORIGIN_TL)
	{
		IplImage * pFlippedImg = cvCreateImage(cvSize(pImage->width,pImage->height), pImage->depth, pImage->nChannels);
		cvFlip(pImage,pFlippedImg, 0);
		pImage = pFlippedImg;
	}

	glGenTextures(1, &id);  
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D,pImage->nChannels,pImage->width,pImage->height,GL_BGR_EXT,GL_UNSIGNED_BYTE,pImage->imageData);  
}

///////////////////////////////////////////////////////////////////////////////////
