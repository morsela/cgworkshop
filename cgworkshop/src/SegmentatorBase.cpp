#include "SegmentatorBase.h"

#include <algorithm>
#include <limits>
#include "GMM/GMM.h"
#include "GMM/kGMM.h"


#include "GraphHandler.h"

using namespace std;

#define USE_ALPHA_EXPANSION 0
#define USE_SOFT_COLORIZATION 0

//#define DISP_SEGMENTATION

//#define DISP_CONF_MAPS

//#define NEW_GMM

SegmentatorBase::SegmentatorBase(IplImage * Img, ScribbleVector & scribbles, int nScribbles) :m_pImg(Img)
{
	m_scribbles = scribbles;
	//FIXME: it is not logical to assume that the bg is a scribble
	m_nScribbles = nScribbles;

	m_pLabels = cvCreateMat(m_pImg->height,m_pImg->width, CV_32F );
	m_pScribbles = cvCreateMat(m_pImg->height,m_pImg->width, CV_32F );

	m_Segmentations = new CvMat*[m_nScribbles];
	
	int i;
	for (i=0;i<m_nScribbles;i++)
		m_Segmentations[i] = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );

	m_Probabilities = new CvMat*[m_nScribbles];
	m_BGProbabilities = new CvMat*[m_nScribbles];

	for (i=0;i<m_nScribbles;i++)
		m_Probabilities[i] = cvCreateMat(m_pImg->height,m_pImg->width, CV_32FC1 );
	for (i=0;i<m_nScribbles;i++)
		m_BGProbabilities[i] = cvCreateMat(m_pImg->height,m_pImg->width, CV_32FC1 );

	m_SegmentCount = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );

	m_pSegImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height),m_pImg->depth,m_pImg->nChannels);
	m_pTempSegImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height),m_pImg->depth,m_pImg->nChannels);
}

SegmentatorBase::~SegmentatorBase()
{
	delete m_pFe;

	cvReleaseImage(&m_pTempSegImg);
	cvReleaseImage(&m_pSegImg);

	for (int i=0;i<m_nScribbles;i++)
	{
		cvReleaseMat(&m_Segmentations[i]),
		cvReleaseMat(&m_Probabilities[i]);
		cvReleaseMat(&m_BGProbabilities[i]);
	}

	cvReleaseMat(&m_pLabels);
	cvReleaseMat(&m_pScribbles);
}

void SegmentatorBase::getMask(CvMat * mask, int label) 
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			int value = (int) cvmGet(m_pLabels,i,j);
			if (value == label) 
				mask->data.ptr[i*m_pImg->width+j]=1;
			else
				mask->data.ptr[i*m_pImg->width+j]=0;
		}
	}
}

void SegmentatorBase::Segment()
{
	m_pFe = new CFeatureExtraction(m_pImg);
	m_pFe->Run();

	cvSet(m_pScribbles, cvScalar(-1), 0);
	for (int scribble=0;scribble<m_nScribbles;scribble++)
	{
		for (int i = 0; i < (int)(m_scribbles[scribble].GetScribbleSize());i++)
		{
			CPointInt pI = m_scribbles[scribble][i];
			int x = pI.x;
			int y = pI.y;

			cvmSet(m_pScribbles, y, x, scribble);
		}	
	}

	printf("Segment\n");
	
	for (int i=0;i<m_nScribbles;i++)
			SegmentOne(i);	
}

void SegmentatorBase::PrintStatus(CvMat ** masks)
{
	printf("+ ----------------\n");
	printf("Current status\n");
	
	for (int i=0;i<m_nScribbles;i++)
	{
		int count = cvNorm(masks[i], NULL, CV_L1);
		printf("	+- Scribble(%d) has %d pixels\n", i, count);
				
	}
	
	printf("- ----------------\n");
}

// Regular segmentation

void SegmentatorBase::SegmentOne(int scribble) 
{
	
	printf("SegmentOne: Segmenting scribble %d\n", scribble);

	
#ifdef NEW_GMM
	CkGMM * f_gmm = new CkGMM(5,1,0.01);
	CkGMM * b_gmm = new CkGMM(5,1,0.01);	
#else	
	CGMM * f_gmm = new CGMM();
	CGMM * b_gmm = new CGMM();
#endif

	CvMat * pChannels = cvCreateMat(m_pFe->GetPrincipalChannels()->rows,m_pFe->GetPrincipalChannels()->cols,m_pFe->GetPrincipalChannels()->type);
	CvMat * pColorChannels = cvCreateMat(m_pFe->GetColorChannels()->rows,m_pFe->GetColorChannels()->cols,m_pFe->GetColorChannels()->type);
	
	double c_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_C, 0);
	double l1_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L1, 0);
	double l2_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L2, 0);
	printf("PChannels matrix c norm = %lf\n", c_norm);
	printf("PChannels matrix l1 norm = %lf\n", l1_norm);
	printf("PChannels matrix l2 norm = %lf\n", l2_norm);
	cvConvertScale(m_pFe->GetPrincipalChannels(), pChannels, 1);
	//cvNormalize(pChannels, pChannels, 0, 50, CV_MINMAX);
	cvConvertScale(m_pFe->GetColorChannels(), pColorChannels, 1);
	//cvNormalize(pColorChannels, pColorChannels, 0, 50, CV_MINMAX);
	
	CvMat * f_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	CvMat * b_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	
	cvSetZero( m_pLabels );
	cvSet( b_mask, cvScalar(1), 0);
	
	cvSetZero( f_mask );
	//get initial foreground mask
	for (int i = 0; i < (int)(m_scribbles[scribble].GetScribbleSize());i++)
	{
		CPointInt pI = m_scribbles[scribble][i];
		int x = pI.x;
		int y = pI.y;
		//printf("%d, %d\n", x,y);
		f_mask->data.ptr[y*m_pImg->width+x]=1;
		b_mask->data.ptr[y*m_pImg->width+x]=0;
	}
			
	//calculate beta
	GraphHandler::calc_beta(m_pImg->height, m_pImg->width, pColorChannels);
	//init GMMs
#ifdef NEW_GMM
	f_gmm->Init(pChannels, f_mask);
	b_gmm->Init(pChannels, b_mask);
#else
	f_gmm->Init(pChannels, f_mask, CvEM::COV_MAT_GENERIC);
	b_gmm->Init(pChannels, b_mask, CvEM::COV_MAT_GENERIC);
#endif
	
	//Sink (Background)
	CvMat * Bu = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );
	//Source (Foreground)
	CvMat * Fu = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	IplImage * outImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height), IPL_DEPTH_8U, 1);
	
	CvMat * conf_map = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );

#ifdef NEW_GMM	
	CvMat * conf_map_fg = new CvMat;
	cvInitMatHeader(conf_map_fg, m_pImg->height, m_pImg->width, CV_32F, f_gmm->GetProbabilities()->data.fl);
	
	CvMat * conf_map_bg = new CvMat;
	cvInitMatHeader(conf_map_bg, m_pImg->height, m_pImg->width, CV_32F, b_gmm->GetProbabilities()->data.fl);
#else	
	CvMat * conf_map_fg = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
	CvMat * conf_map_bg = cvCreateMat( m_pImg->height, m_pImg->width, CV_32F );
#endif	
	char title[50];

	double CurFlow =0, PrevFlow = 0;
	for (int n=0; n < MAX_ITER; n++) {

		GraphHandler *graph = new GraphHandler();
		
		// Get probabilites
		printf("+GMM\n");
#ifdef NEW_GMM	
#else
		f_gmm->GetAllProbabilities(pChannels, conf_map_fg);
		b_gmm->GetAllProbabilities(pChannels, conf_map_bg);
#endif
		printf("-GMM\n");
		// Set weights
		
		printf("+Weights\n");
		
		cvSetZero( Fu );
		cvSetZero( Bu );
		for (int i = 0; i < (int)(m_scribbles[scribble].GetScribbleSize());i++)
		{
			CPointInt pI = m_scribbles[scribble][i];
			int x = pI.x;
			int y = pI.y;
			
			cvmSet(Fu,y,x,10000);
			cvmSet(Bu,y,x,0);
		}
				
		for (int i=0; i<m_pImg->height; i++)
		{
			for (int j=0; j<m_pImg->width; j++)
			{
				if (cvmGet(Fu, i, j) == 0)
				{
					//calcweights
					cvmSet(Fu,i,j,cvmGet(conf_map_bg, i,j));
					cvmSet(Bu,i,j,cvmGet(conf_map_fg, i,j));
				}
			}
		}
		
		printf("-Weights\n");
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
		printf("+Graph cut\n");
		// Graph cut
		graph->init_graph(m_pImg->height, m_pImg->width, pColorChannels);
		graph->assign_weights(Bu, Fu);
		
		graph->do_MinCut(*m_pLabels);
		
		printf("-Graph cut\n");
		
		PrevFlow = CurFlow;
		CurFlow = graph->getFlow();
		if (fabs((CurFlow-PrevFlow)/CurFlow)<TOLLERANCE)
			n = MAX_ITER;

		printf("Flow is %lf\n" ,graph->getFlow());

#ifdef DISP_SEGMENTATION
		// Display segmentation
		cvConvertScale(m_Segmentations[scribble], outImg,255,0); 
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		cvShowImage( title, outImg );
		cvWaitKey(0);
		cvDestroyWindow(title);	
		
		
		IplImage * img = GetSegmentedImage(scribble);
		strcpy(title, "Segmentation");
		cvNamedWindow( title, 1 );
		cvShowImage( title, img );
		cvWaitKey(0);
		cvDestroyWindow(title);				
#endif

		CalcAverage(conf_map_bg, conf_map_fg,scribble);

		// Update GMM
		getMask(f_mask,1);
		getMask(b_mask,0);
		
		printf("+GMM Step\n");
#ifdef NEW_GMM	
		f_gmm->NextStep(f_mask);
		b_gmm->NextStep(b_mask);
#else
		f_gmm->NextStep(pChannels, f_mask, CvEM::COV_MAT_GENERIC);
		b_gmm->NextStep(pChannels, b_mask, CvEM::COV_MAT_GENERIC);	
#endif
		printf("-GMM Step\n");
		delete graph;
	}
	
	cvConvert(m_pLabels, m_Segmentations[scribble]);
	
	// Save the last FG confidence map
	cvConvert(conf_map_fg, m_Probabilities[scribble]);
	cvConvert(conf_map_bg, m_BGProbabilities[scribble]);

}

IplImage * SegmentatorBase::GetSegmentedImage()
{
	return m_pSegImg;	
}

IplImage * SegmentatorBase::GetSegmentedImage(int scribble)
{
	printf("+GetSegmentedImage(%d)\n", scribble);
	cvCvtColor(m_pImg, m_pTempSegImg, CV_BGR2YCrCb);

	uchar * pData  = (uchar *)m_pTempSegImg->imageData;
	
	CvScalar color;
	for (int i=0; i<m_pLabels->rows; i++)
	{
		for (int j=0; j<m_pLabels->cols; j++) 
		{
			int label = -1;
			
			if (USE_ALPHA_EXPANSION)///TODO: lose this shit
				label = (int) cvmGet(m_pLabels, i,j);
			else
			{
				if (cvmGet(m_Segmentations[scribble],i,j))
					label = scribble;
			}
				
			if (label == scribble)
				RGB2YUV(m_scribbles[label].GetColor(), &color);
			else
			{
				color.val[0] = 0;
				color.val[1] = -128;	
				color.val[2] = -128;
			}

			RecolorPixel(pData, i,j, &color);
		}
	}

	cvCvtColor(m_pTempSegImg, m_pTempSegImg, CV_YCrCb2BGR);	
	printf("+GetSegmentedImage(%d)\n", scribble);
	
	return m_pTempSegImg;
}


void SegmentatorBase::RecolorPixel(uchar * pData, int y, int x, CvScalar * pColor)
{
	int step = m_pImg->widthStep;

	// Only take UV components
	pData[y*step+x*3+1] = (uchar)pColor->val[1];
	pData[y*step+x*3+2] = (uchar)pColor->val[2];
}

void SegmentatorBase::RGB2YUV(CvScalar * pRGB, CvScalar * pYUV)
{
	char delta = 128;
	
	// calculate Y from BGR
	// Y = 0.299*R + 0.587*G + 0.114*B 
	uchar luma = 255 * (0.299*pRGB->val[0] + 0.587*pRGB->val[1] + 0.114*pRGB->val[2]);
	pYUV->val[0] = luma;

	// calculate Cr from BGR
	// Cr = (R-Y)*0.713 + delta
	uchar cr = floor((255*pRGB->val[0] - luma)*0.713) + delta;
	pYUV->val[1] = cr;
	
	// calculate Cb from BGR
	// Cb = (B-Y)*0.564 + delta 
	uchar cb = floor((255*pRGB->val[2] - luma)*0.564) + delta;	
	pYUV->val[2] = cb;
}

 
void SegmentatorBase::Colorize()
{
  	// TODO:	
  	printf("+Colorize\n");
  	Segment();
  	AssignColors();
  	printf("-Colorize\n");
}


extern double calcDist(CvMat * smoothness, int i, int j, double beta);

void SegmentatorBase::CalcAverage(CvMat * Bg, CvMat * Fg, int scribble) {

	double E1 =0.0, E2 = 0.0;

	CvMat * Segmentation = m_Segmentations[scribble];

	int cols = Segmentation->cols, rows = Segmentation->rows;
	for (int i=0; i<rows; i++)
	{
		for (int j=0; j<cols; j++) {

			int seg1 = cvmGet(Segmentation, i, j);
			int seg2;


			if (j<cols-1)
			{
				seg2 = cvmGet(Segmentation, i, j+1);
				if (seg1 != seg2)
					E2 += calcDist(m_pFe->GetColorChannels(), i*cols +j, (i)*cols +(j+1), GraphHandler::beta);			
			}
			if (i<rows-1)
			{
				seg2 = cvmGet(Segmentation, i+1, j);
				if (seg1 != seg2)
					E2 += calcDist(m_pFe->GetColorChannels(), i*cols +j, (i+1)*cols +(j), GraphHandler::beta);			
			}
			
			if (seg1==0) //bg
				E1 += (cvmGet(Bg, i, j));
			else //fg 
				E1 += (cvmGet(Fg, i,j));
		}
	}

	printf("----------------\n");	
	printf("E1=%lf, E2=%lf, E1/E2=%lf, E2/E1=%lf\n", E1, E2, E1/E2, E2/E1);
	printf("Flow=%lf ?\n", E1+E2);
	printf("----------------\n");


}

void SegmentatorBase::CountSegments()
{
	cvSetZero( m_SegmentCount );
	
	for (int y = 0; y < m_pImg->height; y++)
	{
		for (int x = 0; x < m_pImg->width; x++)
		{
			int nCount = 0;
			for (int n=0; n<m_nScribbles; n++) 
			{
				if (cvmGet(m_Segmentations[n],y,x))
				{
					nCount++;
				}
			}
			
			cvmSet(m_SegmentCount, y, x, nCount);
		}
	}	
}

void SegmentatorBase::AssignColors()
{	
	printf("Assigning colors......\n");
	CountSegments();
	
	for (int n=0; n<m_nScribbles; n++) 
		cvNormalize(m_Probabilities[n], m_Probabilities[n], 0, 1, CV_MINMAX);

	cvCvtColor(m_pImg, m_pSegImg, CV_BGR2YCrCb);

	uchar * pData  = (uchar *)m_pSegImg->imageData;
	
	CvScalar color;
	for (int i=0; i<m_pLabels->rows; i++)
	{
		for (int j=0; j<m_pLabels->cols; j++) 
		{
			AssignColor(i,j,&color);
			RecolorPixel(pData, i,j, &color);

		}
	}

	cvCvtColor(m_pSegImg, m_pSegImg, CV_YCrCb2BGR);
}



