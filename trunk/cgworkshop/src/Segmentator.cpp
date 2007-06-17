#include "Segmentator.h"

#include <algorithm>

#include "GMM/GMM.h"

#include "GraphHandler.h"

using namespace std;

Segmentator::Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector scribbles) :m_pImg(Img)
{
	m_points = scribbles[0].GetScribblePoints();
	m_pFe = fe;
	m_Segmentation = cvCreateMat(m_pImg->width,m_pImg->height, CV_8UC1 );
}

void Segmentator::getMask(CvMat * mask, int isBackground) 
{

	for (int i=0; i<m_pImg->height; i++)
		for (int j=0; j<m_pImg->width; j++) {
			int value = cvmGet(this->m_Segmentation,i,j);
			if (isBackground) 
				value = 1-value;
			cvmSet(mask,1, i*m_pImg->width+j, value);
		}
}

void Segmentator::Segment() 
{
	CGMM * f_gmm = new CGMM();
	CGMM * b_gmm = new CGMM();

	CvMat * pChannels = m_pFe->GetPrincipalChannels();
	CvMat * f_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	CvMat * b_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );

	cvSetZero( f_mask );
	//get initial foreground mask
	for (int i=0;i< (int)m_points.size();i++)
	{
		CPointInt	pI = m_points[i];
		int x = pI.x;
		int y = pI.y;
		//printf("%d, %d\n", x,y);
		f_mask->data.ptr[y*m_pImg->width+x]=1;
	}
			

	//init GMMs
	f_gmm->Init(pChannels, f_mask);
	b_gmm->Init(pChannels);
	f_gmm->NextStep(pChannels, f_mask);
	b_gmm->NextStep(pChannels);

	//Sink (background)
	CvMat * Tu = cvCreateMat(m_pImg->height, m_pImg->width, CV_8UC1 );
	//Source (foreground)
	CvMat * Su = cvCreateMat( m_pImg->height, m_pImg->width, CV_8UC1 );
	
	CvMat * point = cvCreateMat(1,6,CV_8UC1);
		
	for (int n=0; n<MAX_ITER; n++) {
		GraphHandler *graph = new GraphHandler();
		printf("gmm->NextStep(pTrainMat);\n");
		for (int i=0; i<m_pImg->width; i++)
			for (int j=0; j<m_pImg->height; j++) 
				if (find(m_points.begin(), m_points.end(), CPointInt(i,j))!= m_points.end())
				{//inside scribble
					cvmSet(Tu,i,j,100000);
					cvmSet(Su,i,j,0);
				}
				else
				{
					//get the 6d point
					for (int k=0; k<6; k++)
						cvmSet(point,1,k,cvmGet(pChannels,i,j));
					//calcweights
					cvmSet(Tu,i,j,b_gmm->GetProbability(point));
					cvmSet(Su,i,j,f_gmm->GetProbability(point));
				}
			

		graph->do_MinCut(*m_Segmentation);
		getMask(f_mask,0);
		getMask(b_mask,1);
		f_gmm->NextStep(pChannels, f_mask);
		b_gmm->NextStep(pChannels, b_mask);	
		delete graph;
	}
}