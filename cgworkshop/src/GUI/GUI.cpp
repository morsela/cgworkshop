
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
		// as points
		glBegin( GL_POINTS);
		{
			for ( unsigned int i = 0; i < m_scribbles.size(); i ++)
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

#include "ml.h"

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

	case '.':
		{
		
/*		const CvScalar colors[] = {{{0,0,255}},{{0,255,0}},{{0,255,255}},{{255,255,0}}};
		CvEM em_model;
		CvEMParams params;

		CFeatureExtraction * fe;

		fe = new CFeatureExtraction(m_pImg);
		fe->Run();

		float _sample[2];
    CvMat sample = cvMat( 1, 2, CV_32FC1, _sample );
	
		CvMat* labels = cvCreateMat( m_pImg->width * m_pImg->height, 1, CV_32SC1 );
		IplImage* img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );
		
		  // initialize model's parameters
		params.covs      = NULL;
		params.means     = NULL;
		params.weights   = NULL;
		params.probs     = NULL;
		params.nclusters = 5;
		params.cov_mat_type       = CvEM::COV_MAT_SPHERICAL;
		params.start_step         = CvEM::START_AUTO_STEP;
		params.term_crit.max_iter = 10;
		params.term_crit.epsilon  = 0.1;
		params.term_crit.type     = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;

		CvMat * pChannels = fe->GetTextureChannels();
		// cluster the data
		em_model.train( pChannels, 0, params, labels );

		// classify every image pixel
		cvZero( img );
		for( int i = 0; i < img->height; i++ )
		{
			for( int j = 0; j < img->width; j++ )
			{
				CvPoint pt = cvPoint(j, i);
				pChannels->data.fl[0] = (float)j;
				pChannels->data.fl[1] = (float)i;
				int response = cvRound(em_model.predict( &sample, NULL ));
				CvScalar c = colors[response];

				cvCircle( img, pt, 1, cvScalar(c.val[0]*0.75,c.val[1]*0.75,c.val[2]*0.75), CV_FILLED );
			}
		}


		cvNamedWindow( "EM-clustering result", 1 );
		cvShowImage( "EM-clustering result", img );
		cvWaitKey(0);

*/
		}
		break;

	case 'r':
		{

		CFeatureExtraction * fe;

		fe = new CFeatureExtraction(m_pImg);
		fe->Run();


		Segmentator seg(m_pImg, fe, m_scribbles);

		//GraphHandler::init_graph(m_pImg->height, m_pImg->width, fe->GetColorChannels());

 		seg.Segment();

		// display
		IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
		cvConvertScale(seg.getSegmentation(),outImg,255,0); 	
		
		char title[50];
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		cvShowImage( title, outImg );
		cvWaitKey(0);
		cvDestroyWindow(title);	
		
		cvSaveImage("test.bmp",outImg);
		cvReleaseImage(&outImg);

		}
		break;		
	default:
		break;
	}
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
			//FIXME: id should not be constant
			int id = 1;
			
			CScribble scribble(id);

			m_scribbles.push_back( scribble );

			AddScribblePoints(x,y);
			
			m_fScribbling = true;
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
	int height = m_pImg->height;
	int width  = m_pImg->width;

	float ratio_x = (float) width / GetWidth();
	float ratio_y = (float)	height / GetHeight();

	CPointInt pI = CPointInt( (int)(ratio_x * x), (int)((GetHeight() - y) * ratio_y));

	// the start of the vector
	CPointFloat pF = CPointFloat( m_orthoBBox[0] * ( 1 - 2 * (float)x / GetWidth() ),
								m_orthoBBox[3] * ( 1 - 2 * (float)y / GetHeight() ));

	m_scribbles.back().AddImagePoint (pI);
	m_scribbles.back().AddObjectPoint(pF);
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
