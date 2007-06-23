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
			int value = (int) cvmGet(this->m_Segmentation,i,j);
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
		f_mask->data.ptr[(m_pImg->height - y)*m_pImg->width+x]=1;
	}
			

	//init GMMs
	f_gmm->Init(pChannels, f_mask);
	b_gmm->Init(pChannels);
	f_gmm->NextStep(pChannels, f_mask);
	b_gmm->NextStep(pChannels);

	//Sink (Background)
	CvMat * Bu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );
	//Source (Foreground)
	CvMat * Fu = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	
	CvMat * point = cvCreateMat(1,6,CV_32F);
	for (int n=0; n<MAX_ITER; n++) {
		double s1 =0, s2=0;
		int x=0;
		GraphHandler *graph = new GraphHandler();
		printf("gmm->NextStep(pTrainMat);\n");
		for (int i=0; i<m_pImg->height; i++)
			for (int j=0; j<m_pImg->width; j++)                     
				if (find(m_points.begin(), m_points.end(), CPointInt(j,m_pImg->height-i))!= m_points.end())
				{//inside scribble
					cvmSet(Fu,i,j,100000);
					cvmSet(Bu,i,j,0);
				
				}
				else
				{
					//get the 6d point
					cvGetRow(pChannels, point, i*m_pImg->width+j);

					//calcweights
					x++;
					s1+=-log(b_gmm->GetProbability(point));
					s2+=-log(f_gmm->GetProbability(point));
					cvmSet(Bu,i,j,-log(b_gmm->GetProbability(point)));
					cvmSet(Fu,i,j,-log(f_gmm->GetProbability(point)));
				}
				
			
		printf("%lf %lf \n", s1/x, s2/x);
		graph->init_graph(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels());
		graph->assign_weights(Bu, Fu);
		
		graph->do_MinCut(*m_Segmentation);
		
		printf("Flow is %lf\n" ,graph->getFlow());

		getMask(f_mask,0);
		getMask(b_mask,1);
		f_gmm->NextStep(pChannels, f_mask);
		b_gmm->NextStep(pChannels, b_mask);	
		delete graph;
	}

}


