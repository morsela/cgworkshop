#include "Segmentator.h"

#include <algorithm>

#include "GMM/GMM.h"

#include "GraphHandler.h"

using namespace std;

Segmentator::Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector scribbles) :m_pImg(Img)
{
	m_points = scribbles[0].GetScribblePoints();
	m_pFe = fe;
	m_Segmentation = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );
}

void Segmentator::getMask(CvMat * mask, int isBackground) 
{

	for (int i=0; i<m_pImg->height; i++)
		for (int j=0; j<m_pImg->width; j++) {
			int value = cvmGet(this->m_Segmentation,i,j);
			if (isBackground) 
				value = 1-value;
			mask->data.ptr[i*m_pImg->width+j]=value;
			
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
		f_mask->data.ptr[y*m_pImg->width+m_pImg->height-x]=1;
	}
			

	//init GMMs
	f_gmm->Init(pChannels, f_mask);
	b_gmm->Init(pChannels);
	f_gmm->NextStep(pChannels, f_mask);
	b_gmm->NextStep(pChannels);

	//Sink (background)
	CvMat * Tu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );
	//Source (foreground)
	CvMat * Su = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	
	CvMat * point = cvCreateMat(1,6,CV_32F);
	
	for (int n=0; n<MAX_ITER; n++) {
		GraphHandler *graph = new GraphHandler();
		printf("gmm->NextStep(pTrainMat);\n");
		for (int i=0; i<m_pImg->height; i++)
			for (int j=0; j<m_pImg->width; j++)                     
				if (find(m_points.begin(), m_points.end(), CPointInt(j,m_pImg->height-i))!= m_points.end())
				{//inside scribble
					cvmSet(Tu,i,j,100000);
					cvmSet(Su,i,j,0);
				
				}
				else
				{
					//get the 6d point
					for (int k=0; k<6; k++)
						cvmSet(point,0,k,cvmGet(pChannels,i*m_pImg->width+j,k));
					//calcweights
					cvmSet(Tu,i,j,-log(b_gmm->GetProbability(point)));
					cvmSet(Su,i,j,-log(f_gmm->GetProbability(point)));
				}
				
			
	
		graph->init_graph(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels());
		graph->assign_weights(Tu, Su);
		graph->do_MinCut(*m_Segmentation);
		//std::cout << "Flow is "<< graph->getFlow()<<endl;
		getMask(f_mask,0);
		getMask(b_mask,1);
		f_gmm->NextStep(pChannels, f_mask);
		b_gmm->NextStep(pChannels, b_mask);	
		delete graph;
	}

}