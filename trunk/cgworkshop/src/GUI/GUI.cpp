
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

#include <highgui.h>

#include "GUI.h"

#include "../fe/FeatureExtraction.h"
#include "../GMM/GMM.h"
#include "../GraphHandler.h"
#include "../Segmentator.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *image, unsigned int &id );

///////////////////////////////////////////////////////////////////////////////////

CGUI::CGUI() 
{ 
	m_fScribbling		= false;
	m_nCurScribble		= UNDEFINED;
	m_nScribblesNum		= -1;
}

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
		glVertex3f( m_orthoBBox[2] - m_ctrlPanel.GetWidth(), m_orthoBBox[1], 0);
		glTexCoord2f( 1.0, 1.0 );
		glVertex3f( m_orthoBBox[2] - m_ctrlPanel.GetWidth(), m_orthoBBox[3], 0);
	}
	glEnd();

	glDisable( GL_TEXTURE_2D );

	glBegin(GL_POINTS);
	{
		for (unsigned int i = 0; i < m_scribbles.size(); i ++)
		{
			pColor = m_scribbles[i].GetColor();
			glColor3f(pColor->val[0],pColor->val[1],pColor->val[2]);
			m_scribbles[i].Draw();
		}
	}
	glEnd();	

	m_ctrlPanel.Draw();

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

	m_nWindowWidth = x;

	m_nWindowHeight = y;

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

	m_ctrlPanel.Setup(m_orthoBBox[2] - m_ctrlPanel.GetWidth(), m_orthoBBox[3], m_nWindowHeight);

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
		{
		m_loader.Load(m_scribbles);
		
		int nScribbles = 0;
		for (unsigned int i = 0; i < m_scribbles.size() ; i++)
			if (m_scribbles[i].IsValid())
			{
				printf("m_scribbles[i]=%d\n", i, nScribbles);
				nScribbles++;
			}

		printf("nScribbles=%d\n", nScribbles);

		m_nScribblesNum		= nScribbles;
		m_nCurScribble		= UNDEFINED;
		} break;
		

	case 's':
		m_loader.Save(m_scribbles);
		break;

	case 'c':
		//m_ctrlPanel.Reset();
		for (unsigned int i = 0; i < m_scribbles.size(); i++)
			m_scribbles[i].Reset();
		m_nScribblesNum		= -1;
		m_nCurScribble		= UNDEFINED;
		break;

	case 'r':
		{

		int nScribbles = 0;

		//Why do we need this?
		for (unsigned int i = 0; i < m_scribbles.size() ; i++)
			if (m_scribbles[i].IsValid())
				nScribbles++;

		printf("nScribbles=%d\n", nScribbles);

		Segmentator seg(m_pImg, m_scribbles, nScribbles);

 		seg.Colorize();

		// display
		//IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
		//cvConvertScale(seg.getSegmentation(),outImg,255,0); 	
		
		char title[50];
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		//cvShowImage( title, outImg );
		char name[10] = "testi.bmp";
		for (int i=0;i<nScribbles;i++)
		{
			cvNamedWindow( title, 1 );
			cvShowImage( title, seg.GetSegmentedImage(i) );
			cvWaitKey(0);
			cvDestroyWindow(title);	
			
			name[4] = i+ '0';
			cvSaveImage(name,seg.GetSegmentedImage(i));
		}
			cvNamedWindow( title, 1 );
			cvShowImage( title, seg.GetSegmentedImage() );
			cvWaitKey(0);
			cvDestroyWindow(title);
			name[4]='f';
			cvSaveImage(name,seg.GetSegmentedImage());
		

		}
		break;		
	default:
		break;
	}

	/*if (key >= '0' && key <= '9')
		m_nCurScribble = key - 48;*/
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
			//check if the left click was on the control panel side
			if (m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWindowWidth() ) > m_orthoBBox[2] - m_ctrlPanel.GetWidth())
				m_ctrlPanel.Choose(m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWindowWidth()),
									m_orthoBBox[3] * ( 1 - 2 * (float)y / GetWindowHeight()));
			else
			{
				CvScalar color;
				int nCurScribble = m_ctrlPanel.GetChosenColor(color);
				if (nCurScribble != UNDEFINED)
				{
					if (nCurScribble != m_nCurScribble)
					{
						m_nScribblesNum++;
						printf("m_nScribblesNum=%d\n", m_nScribblesNum);
						m_scribbles[m_nScribblesNum].SetID(nCurScribble);
						m_scribbles[m_nScribblesNum].SetColor(&color);
						m_nCurScribble = nCurScribble;
					}
					
					AddScribblePoints(x,y);
					m_fScribbling = true;
				}
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
	int nLineWidth;

	int height = m_pImg->height;
	int width  = m_pImg->width;

	float ratio_x = (float) (m_ctrlPanel.GetWidth()/2 + GetImageWidth()) / GetWindowWidth();
	float ratio_y = (float)	GetImageHeight() / GetWindowHeight();

	m_ctrlPanel.GetChosenLineWidth(nLineWidth);
	nLineWidth = (nLineWidth - 1) / 2;
	for (int i = x - nLineWidth; i <= x + nLineWidth; i++)
	{
		for (int j = y - nLineWidth; j <= y + nLineWidth; j++)
		{
			if( m_pImg->origin == IPL_ORIGIN_TL)
				origY = j;
			else
				origY = GetImageHeight() - j;

			if (m_orthoBBox[0] * ( 1 - 2 * (float)i / GetWindowWidth() ) > m_orthoBBox[2] - m_ctrlPanel.GetWidth())
			{
				printf("Out of bounds.");
				continue;
			}

			CPointInt pI = CPointInt( (int)(ratio_x * i), (int)(origY * ratio_y));

			// the start of the vector
			CPointFloat pF = CPointFloat( m_orthoBBox[0] * ( 1 - 2 * (float)i / GetWindowWidth() ),
										m_orthoBBox[3] * ( 1 - 2 * (float)j / GetWindowHeight() ));

			m_scribbles[m_nScribblesNum].AddImagePoint (pI);
			m_scribbles[m_nScribblesNum].AddObjectPoint(pF);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////

bool CGUI::Setup(char * pszImagePath, char *pScribbleFile /* = NULL */)
{
	// Load the input image
	m_pImg = cvLoadImage(pszImagePath,1);
	if (m_pImg == NULL)
		return false;
	
	SetImageSize(m_pImg->width, m_pImg->height);
	SetWindowSize(m_pImg->width + m_ctrlPanel.GetWidth(), m_pImg->height);

	if (pScribbleFile)
		m_loader.Setup(pScribbleFile);

	m_nCurScribble = UNDEFINED;
	m_scribbles.resize(SCRIBBLE_NUMBER);
	for (int i = 0; i < SCRIBBLE_NUMBER; i++)
	{
		m_scribbles[i].SetID(i);
		m_scribbles[i].SetColor(&s_colors[i]);
	}
	return true;
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
