#include "Segmentator.h"

#include <algorithm>

#include "GMM/GMM.h"

#include "GraphHandler.h"

using namespace std;

#define DISP_SEGMENTATION

Segmentator::Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector scribbles) :m_pImg(Img)
{
	m_points = scribbles[0].GetScribblePoints();
	m_pFe = fe;
	m_Segmentation = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );
	m_pSegImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height),m_pImg->depth,m_pImg->nChannels);
}

Segmentator::~Segmentator()
{
	cvReleaseImage(&m_pSegImg);
}

void Segmentator::getMask(CvMat * mask, int isBackground) 
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			int value = (int) cvmGet(this->m_Segmentation,i,j);
			if (isBackground) 
				value = 1-value;
			mask->data.ptr[i*m_pImg->width+j]=value;
		}
	}
}

void Segmentator::Segment() 
{
	CGMM * f_gmm = new CGMM();
	CGMM * b_gmm = new CGMM();

	CvMat * pChannels = cvCreateMat(m_pFe->GetPrincipalChannels()->rows,m_pFe->GetPrincipalChannels()->cols,m_pFe->GetPrincipalChannels()->type);
	cvConvertScale(m_pFe->GetPrincipalChannels(), pChannels, 0.2);

	CvMat * f_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	CvMat * b_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	
	cvSetZero( m_Segmentation );
	getMask(b_mask,1);
	
	
	cvSetZero( f_mask );
	//get initial foreground mask
	for (int i = 0; i < (int)m_points.size();i++)
	{
		CPointInt pI = m_points[i];
		int x = pI.x;
		int y = pI.y;
		//printf("%d, %d\n", x,y);
		f_mask->data.ptr[y*m_pImg->width+x]=1;
//		b_mask->data.ptr[y*m_pImg->width+x]=0;
	}
			

	//init GMMs
	f_gmm->Init(pChannels, f_mask, CvEM::COV_MAT_DIAGONAL);
	b_gmm->Init(pChannels, b_mask, CvEM::COV_MAT_DIAGONAL);
	
	//Sink (Background)
	CvMat * Bu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );
	//Source (Foreground)
	CvMat * Fu = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
	
	CvMat * conf_map = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	CvMat * conf_map_fg = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	CvMat * conf_map_bg = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	char title[50];

	for (int n=0; n < MAX_ITER; n++) {

		GraphHandler *graph = new GraphHandler();
		
		// Get probabilites
		f_gmm->GetAllProbabilities(pChannels, conf_map_fg);
		b_gmm->GetAllProbabilities(pChannels, conf_map_bg);
		
		// Set weights
		for (int i=0; i<m_pImg->height; i++)
		{
			for (int j=0; j<m_pImg->width; j++)
			{
				if (find(m_points.begin(), m_points.end(), CPointInt(j,i))!= m_points.end())
				{//inside scribble
					cvmSet(Fu,i,j,10000);
					cvmSet(Bu,i,j,0);
				
				}
				else
				{
					//calcweights
					cvmSet(Fu,i,j,-1*log(cvmGet(conf_map_bg, i,j)));
					cvmSet(Bu,i,j,-1*log(cvmGet(conf_map_fg, i,j)));
				}
			}
		}
		
#ifdef DISP_CONF_MAPS
		// Display FG conf map
		//cvConvertScale(conf_map_fg, outImg,255,0); 
		strcpy(title, "fg-conf-map");
		cvNamedWindow( title, 1 );
		cvShowImage( title, conf_map_fg );
		cvWaitKey(0);
		cvDestroyWindow(title);	
		
		// Display BG conf map
		//cvConvertScale(conf_map_bg, outImg,255,0); 
		strcpy(title, "bg-conf-map");
		cvNamedWindow( title, 1 );
		cvShowImage( title, conf_map_bg );
		cvWaitKey(0);
		cvDestroyWindow(title);	
#endif

		// Graph cut
		graph->init_graph(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels());
		graph->assign_weights(Bu, Fu);
		
		graph->do_MinCut(*m_Segmentation);

		printf("Flow is %lf\n" ,graph->getFlow());

#ifdef DISP_SEGMENTATION
		// Display segmentation
		cvConvertScale(m_Segmentation, outImg,255,0); 
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		cvShowImage( title, outImg );
		cvWaitKey(0);
		cvDestroyWindow(title);	
		
		
		IplImage * img = GetSegmentedImage();
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		cvShowImage( title, img );
		cvWaitKey(0);
		cvDestroyWindow(title);				
#endif

		// Update GMM
		getMask(f_mask,0);
		getMask(b_mask,1);
		f_gmm->NextStep(pChannels, f_mask, CvEM::COV_MAT_DIAGONAL);
		b_gmm->NextStep(pChannels, b_mask, CvEM::COV_MAT_DIAGONAL);	
		
		delete graph;
	}

}

IplImage * Segmentator::GetSegmentedImage()
{
	int step = m_pImg->widthStep;
	memcpy((uchar *)m_pSegImg->imageData,m_pImg->imageData, m_pImg->imageSize);

	uchar * pData  = (uchar *)m_pSegImg->imageData;

	for (int y = 0; y < m_pImg->height; y++)
	{
		for (int x = 0; x < m_pImg->width; x++)
		{
			int value = (int) cvmGet(this->m_Segmentation,y,x);

			if (value == 1)
				//make segmented foreground image yellow
				pData[y*step+x*3] = 0;
			else
			{
				pData[y*step+x*3+1] = 0;
				//make segmented background image blue
				pData[y*step+x*3+2] = 0;
			}
		}
	}

	return m_pSegImg;
}

