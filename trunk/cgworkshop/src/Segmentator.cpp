#include "Segmentator.h"

#include <algorithm>

#include "GMM/GMM.h"
#include "GMM/kGMM.h"

#include "GraphHandler.h"

using namespace std;

#define DISP_SEGMENTATION

#define DISP_CONF_MAPS

//#define NEW_GMM

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

void Segmentator::getMask(CvMat * mask, int nScribble) 
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			int value = (int) cvmGet(this->m_Segmentation,i,j);
			if (value == nScribble)
				mask->data.ptr[i*m_pImg->width+j]=1;
			else
				mask->data.ptr[i*m_pImg->width+j]=0;
		}
	}
}

void Segmentator::getDoubleMask(CvMat * mask, int nScribble1, int nScribble2) 
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			int value = (int) cvmGet(this->m_Segmentation,i,j);
			if ((value == nScribble1) || (value == nScribble2))
				mask->data.ptr[i*m_pImg->width+j]=1;
			else
				mask->data.ptr[i*m_pImg->width+j]=0;
		}
	}
}

void Segmentator::UpdateSegmentation(CvMat * pPartialSeg, int nScribble1, int nScribble2, CvMat * pDoubleMask)
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			// Was included in the partial segmentation
			if (pDoubleMask->data.ptr[i*m_pImg->width+j])
			{
				if (cvmGet(pPartialSeg, i,j) == 1)
					cvmSet(m_Segmentation, i,j, nScribble1);
				else
					cvmSet(m_Segmentation, i,j, nScribble2);
			}
		}
	}	
}

void Segmentator::Segment() 
{
	// NEW!!!
	// TODO: make this a parameter
	int nScribbles = 2;
	
	int bgScribble = nScribbles-1;
	
	int i,j;
	
#ifdef NEW_GMM
	CkGMM ** pGMM = new CkGMM*[nScribbles];
	
	for (i=0;i<nScribbles;i++)
		pGMM[i] = new CkGMM(5,1,0.01);	
#else	
	CGMM ** pGMM = new CGMM*[nScribbles];
	
	for (i=0;i<nScribbles;i++)
		pGMM[i] = new CGMM();
#endif

	CvMat * pChannels = cvCreateMat(m_pFe->GetPrincipalChannels()->rows,m_pFe->GetPrincipalChannels()->cols,m_pFe->GetPrincipalChannels()->type);
	double c_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_C, 0);
	double l1_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L1, 0);
	double l2_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L2, 0);
	printf("PChannels matrix c norm = %lf\n", c_norm);
	printf("PChannels matrix l1 norm = %lf\n", l1_norm);
	printf("PChannels matrix l2 norm = %lf\n", l2_norm);
	cvConvertScale(m_pFe->GetPrincipalChannels(), pChannels, 0.02);

	CvMat * pPartialSeg = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );

	CvMat ** pMasks = new CvMat*[nScribbles];
	
	for (i=0;i<nScribbles;i++)
		pMasks[i] = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	
	CvMat * pDoubleMask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	
	cvSetZero( m_Segmentation );
	
	// Set initial bg mask to 1
	cvSet( pMasks[bgScribble] , cvScalarAll(1));

	//get initial mask for each of the FG scribbles

	for (j=0;j<bgScribble;j++)
	{
		cvSetZero( pMasks[j] );
		// TODO: Get point vector for scribble j
		for (int i = 0; i < (int)m_points.size();i++)
		{
			CPointInt pI = m_points[i];
			int x = pI.x;
			int y = pI.y;

			// Set '1' in scribble mask, '0' in bg mask
			pMasks[j]->data.ptr[y*m_pImg->width+x]=1;
			pMasks[bgScribble]->data.ptr[y*m_pImg->width+x]=0;
		}
	}
			
	//calculate beta
	GraphHandler::calc_beta(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels());
	//init GMMs
#ifdef NEW_GMM
	for (i=0;i<nScribbles;i++)
		pGMM[i]->Init(pChannels, pMasks[i]);
#else
	for (i=0;i<nScribbles;i++)
		pGMM[i]->Init(pChannels, pMasks[i], CvEM::COV_MAT_GENERIC);
#endif

	CvMat * Bu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );
	CvMat * Fu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );

	IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
	
	CvMat ** pConfMaps = new CvMat*[nScribbles];
	
#ifdef NEW_GMM	
	for (i=0;i<nScribbles;i++)
	{
		pConfMaps[i] = new CvMat;
		cvInitMatHeader(pConfMaps[i], m_pImg->height, m_pImg->width, CV_32F, f_gmm->GetProbabilities()->data.fl);
	}

#else	
	for (i=0;i<nScribbles;i++)
		pConfMaps[i] = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
#endif	
	char title[50];

	double CurFlow =0, PrevFlow = 0;
	for (int n=0; n < MAX_ITER; n++) {

		//GraphHandler *graph = new GraphHandler();
		
		// Get probabilites
#ifdef NEW_GMM	
#else
		for (int i=0;i<nScribbles;i++)
			pGMM[i]->GetAllProbabilities(pChannels, pConfMaps[i]);
#endif

		printf("Performing alpha beta swap\n");
		// Graph cuts - One for every pair of labels
		// TODO: This should be in a loop, when the flow doesn't improve anymore, we're done.
		for (int i=0;i<nScribbles;i++)
		{
			for (int j=i+1;j<nScribbles;j++)
			{
				int nScribble1 = i;
				int nScribble2 = j;
				
				// A single graph cut
				
				printf("Setting weights for labels (%d,%d)\n", nScribble1,nScribble2);
				for (int i=0; i<m_pImg->height; i++)
				{
					for (int j=0; j<m_pImg->width; j++)
					{
						// TODO: points for Scribble 1
						if (find(m_points.begin(), m_points.end(), CPointInt(j,i))!= m_points.end())
						{//inside scribble
							cvmSet(Fu,i,j,10000);
							cvmSet(Bu,i,j,0);
								
						}
						/*
						// TODO: points for Scribble 2
						else if (find(m_points.begin(), m_points.end(), CPointInt(j,i))!= m_points.end())
						{//inside scribble
							cvmSet(Bu,i,j,10000);
							cvmSet(Fu,i,j,0);
								
						}
						*/
												
						else
						{
							cvmSet(Bu,i,j,-1*log(cvmGet(pConfMaps[nScribble1], i,j)));	
							cvmSet(Fu,i,j,-1*log(cvmGet(pConfMaps[nScribble2], i,j)));	
						}
					}
				}				
				
				printf("Running min-cut for labels (%d,%d)\n", nScribble1,nScribble2);
				getDoubleMask(pDoubleMask, nScribble1,nScribble2);
				
				GraphHandler *graph = new GraphHandler();
				graph->init_graph(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels(), pDoubleMask);
				graph->assign_weights(Bu, Fu);
				
				graph->do_MinCut(*pPartialSeg);
				
				printf("Flow[%d,%d] is %lf\n" ,nScribble1,nScribble2,graph->getFlow());
				
				delete graph;
				
				printf("Updating segmentation....\n");
				// TODO: Check if the partial segmentation increases total graph flow.....
				UpdateSegmentation(pPartialSeg, nScribble1,nScribble2, pDoubleMask);
			}
		}

#ifdef DISP_SEGMENTATION
		// Display segmentation
		cvConvertScale(m_Segmentation, outImg,255/(nScribbles-1),0); 
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
		for (int i=0;i<nScribbles;i++)
			getMask(pMasks[i],i);
		
#ifdef NEW_GMM	
		for (int i=0;i<nScribbles;i++)
			pGMM[i]->NextStep(pMasks[i]);
#else
		for (int i=0;i<nScribbles;i++)
			pGMM[i]->NextStep(pChannels, pMasks[i], CvEM::COV_MAT_DIAGONAL);
#endif
			
		//delete graph;
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

			if (value == 0)
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

