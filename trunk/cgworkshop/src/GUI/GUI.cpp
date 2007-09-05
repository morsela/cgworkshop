
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

#include <highgui.h>

#include "GUI.h"

#include "../fe/FeatureExtraction.h"
#include "../GMM/GMM.h"
#include "../GraphHandler.h"
#include "../SegmentatorBase.h"
#include "../OneColSegmentator.h"
#include "../AvgColSegmentator.h"
#include "../AEColSegmentator.h"

#ifdef WIN32
#include <process.h>
#endif

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void loadTexture( const IplImage *image, unsigned int &id );

///////////////////////////////////////////////////////////////////////////////////

CGUI::CGUI() 
{ 
	m_fScribbling		= false;
	m_nCurScribble		= UNDEFINED;
	m_nScribblesNum		= -1;
	m_fRunning			= false;
	m_nSegMethod		= ONE_COL_SEGMENTATION;
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
		glVertex3f( m_orthoBBox[0], m_orthoBBox[1] + m_buttonPanel.GetHeight(), 0);
		glTexCoord2f( 1.0, 0.0 );
		glVertex3f( m_orthoBBox[2] - m_ctrlPanel.GetWidth(), m_orthoBBox[1] + m_buttonPanel.GetHeight(), 0);
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
	m_buttonPanel.Draw();

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

	// the orthogonal bounding box
	m_orthoBBox[0] = 0;						// MIN X
	m_orthoBBox[1] = 0;						// MIN Y
	m_orthoBBox[2] = m_nWindowWidth;		// MAX X
	m_orthoBBox[3] = m_nWindowHeight;		// MAX Y

	m_ctrlPanel.Setup(m_orthoBBox[2] - m_ctrlPanel.GetWidth(), m_orthoBBox[3], m_nWindowHeight);
	m_buttonPanel.Setup(m_orthoBBox[0], m_orthoBBox[1] + m_buttonPanel.GetHeight(), m_orthoBBox[2] - m_ctrlPanel.GetWidth());

	gluOrtho2D( 0, m_nWindowWidth, 0, m_nWindowHeight );
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::KeysAction( unsigned char key, int x, int y )
{
	switch( key )
	{
	case 'q':
		glDeleteTextures(1, &m_textures[0]);
		//all threads are terminated when calling exit
		//TODO: won't this cause a memory leak is segmentation is still running?
		exit(0);
		break;

	case 'l':
		{
		m_loader.Load(m_scribbles);
		
		int nScribbles = 0;
		for (unsigned int i = 0; i < m_scribbles.size() ; i++)
			if (m_scribbles[i].IsValid())
				nScribbles++;

		printf("nScribbles=%d\n", nScribbles);

		m_nScribblesNum		= nScribbles - 1;
		if (nScribbles > 0)
			m_nCurScribble		= m_scribbles[m_nScribblesNum].GetID();
		else
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
			if (m_fRunning)
			{
				printf("Segmentation is already taking place\n");
				return;
			}

			m_fRunning = true;

#ifdef WIN32
			_beginthread(ThreadRunSegmentation, 0, this);
#else
			RunSegmentation();
#endif
		} break;	

	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::ThreadRunSegmentation( void *p )
{
	CGUI * pMe = (CGUI *) p;
	pMe->RunSegmentation();
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::RunSegmentation()
{
	int nScribbles = 0;
	SegmentatorBase * seg;

	//Why do we need this?
	for (unsigned int i = 0; i < m_scribbles.size() ; i++)
		if (m_scribbles[i].IsValid())
			nScribbles++;

	printf("nScribbles=%d\n", nScribbles);

	//if we have only one color, colorization is out of the picture
	if (nScribbles == 1)
		seg = new OneColSegmentator(m_pImg, m_scribbles, nScribbles);
	else
	{
		switch (m_nSegMethod)
		{
		case ONE_COL_SEGMENTATION : seg = new OneColSegmentator(m_pImg, m_scribbles, nScribbles); break;
		case AE_COL_SEGMENTATION : seg = new AEColSegmentator(m_pImg, m_scribbles, nScribbles); break;
		case AVG_COL_SEGMENTATION : seg = new AvgColSegmentator(m_pImg, m_scribbles, nScribbles); break;
		}
	}
//	SegmentatorBase * seg =  new OneColSegmentator(m_pImg, m_scribbles, nScribbles);
//	SegmentatorBase * seg =  new AEColSegmentator(m_pImg, m_scribbles, nScribbles);
//	SegmentatorBase * seg =  new AvgColSegmentator(m_pImg, m_scribbles, nScribbles);
	seg->Colorize();

	// display
	//IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
	//cvConvertScale(seg.getSegmentation(),outImg,255,0); 	

	char title[50];
	strcpy(title, "Segmentation of Scribble i");
	//cvNamedWindow( title, 1 );
	//cvShowImage( title, outImg );
	char name[10] = "testi.bmp";
	for (int i=0;i<nScribbles;i++)
	{
		title[25]=i+'0';
		cvNamedWindow( title, 1 );
		cvShowImage( title, seg->GetSegmentedImage(i) );
		cvWaitKey(0);
		cvDestroyWindow(title);	

		name[4] = i+ '0';
		cvSaveImage(name,seg->GetSegmentedImage(i));
	}
	strcpy(title, "Final Segmentation");
	cvNamedWindow( title, 1 );
	cvShowImage( title, seg->GetSegmentedImage() );
	cvWaitKey(0);
	cvDestroyWindow(title);
	name[4]='f';
	cvSaveImage(name,seg->GetSegmentedImage());

	delete seg;
	m_fRunning = false;

#ifdef WIN32
	_endthread();
#endif
}

///////////////////////////////////////////////////////////////////////////////////

void CGUI::MouseAction(int button, int state, int x, int y)
{
	switch ( button )
	{
	case GLUT_LEFT_BUTTON:
		
		// released the left button
		if ( state == GLUT_UP )
		{
			if (m_fScribbling)
				m_fScribbling = false;

			//un-choose all buttons
			m_buttonPanel.Choose(-1,-1);
		} 	
			// pressed the left button
		if ( state == GLUT_DOWN )
		{
			//check if the left click was on the control panel side
			if (x > GetWindowWidth() - m_ctrlPanel.GetWidth())
				m_ctrlPanel.Choose(x,GetWindowHeight()-y);
			else if ( GetWindowHeight()-y < m_buttonPanel.GetHeight())
			{
				CButtonBox::EButtonCommand cmd;
				unsigned char ch;
				m_buttonPanel.Choose(x, GetWindowHeight() - y);
				
				m_buttonPanel.GetChosenButton(cmd);
				switch (cmd)
				{
					case CButtonBox::command_Clear : ch = 'c'; break;
					case CButtonBox::command_Colorize_A : m_nSegMethod = ONE_COL_SEGMENTATION ; ch = 'r'; break;
					case CButtonBox::command_Colorize_B : m_nSegMethod = AE_COL_SEGMENTATION ; ch = 'r'; break;
					case CButtonBox::command_Colorize_C : m_nSegMethod = AVG_COL_SEGMENTATION ; ch = 'r'; break;
					case CButtonBox::command_Load : ch = 'l'; break;
					case CButtonBox::command_Save : ch = 's'; break;
					case CButtonBox::command_Quit : ch = 'q'; break;
				}
				KeysAction(ch,0,0);
			}
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

			if ( i > GetWindowWidth() - m_ctrlPanel.GetWidth()
				|| (GetWindowHeight() - j) < m_buttonPanel.GetHeight() )
			{
				printf("Out of bounds.");
				continue;
			}

			CPointInt pI = CPointInt( (int)(i), (int)(origY));

			// the start of the vector
			CPointFloat pF = CPointFloat( i, GetWindowHeight() - j );

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
	SetWindowSize(m_pImg->width + m_ctrlPanel.GetWidth(), m_pImg->height + m_buttonPanel.GetHeight());

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
