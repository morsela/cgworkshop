#include "Segmentator.h"

#include <algorithm>
#include <limits>
#include "GMM/GMM.h"
#include "GMM/kGMM.h"

#include "GraphHandler.h"

using namespace std;

//#define DISP_SEGMENTATION

//#define DISP_CONF_MAPS

//#define NEW_GMM

Segmentator::Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector & scribbles, int nScribbles) :m_pImg(Img)
{
	m_scribbles = scribbles;
	//FIXME: it is not logical to assume that the bg is a scribble
	m_nScribbles = nScribbles;

	m_pFe = fe;
	
	m_Segmentations = new CvMat*[m_nScribbles];
	
	int i;
	for (i=0;i<m_nScribbles;i++)
		m_Segmentations[i] = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );

	m_Probabilities = new CvMat*[m_nScribbles];
	
	for (i=0;i<m_nScribbles;i++)
		m_Probabilities[i] = cvCreateMat(m_pImg->height,m_pImg->width, CV_32FC1 );
		
	m_FinalSeg = cvCreateMat(m_pImg->height,m_pImg->width, CV_64FC1 );
	cvSetZero( m_FinalSeg );

	m_pSegImg = cvCreateImage(cvSize(m_pImg->width,m_pImg->height),m_pImg->depth,m_pImg->nChannels);
}

Segmentator::~Segmentator()
{
	cvReleaseImage(&m_pSegImg);
	for (int i=0;i<m_nScribbles;i++)
		cvReleaseMat(&m_Segmentations[i]),
		cvReleaseMat(&m_Probabilities[i]);

		
}

void Segmentator::getMask(CvMat * segmentation, CvMat * mask, int isBackground) 
{
	for (int i=0; i<m_pImg->height; i++)
	{
		for (int j=0; j<m_pImg->width; j++) 
		{
			int value = (int) cvmGet(segmentation,i,j);
			if (isBackground) 
				value = 1-value;
			mask->data.ptr[i*m_pImg->width+j]=value;
		}
	}
}

void Segmentator::Segment()
{
	printf("Segment\n");
	int i;
	for (i=0;i<m_nScribbles;i++)
		SegmentOne(i);
}

bool Segmentator::IsInScribble(int i, int j, int scribble)
{
	return m_scribbles[scribble].Find(CPointInt(j,i));
}

bool Segmentator::IsInScribble(int i, int j)
{
	int scribble;
	for (scribble=0;scribble<m_nScribbles;scribble++)
	{
		if (IsInScribble(i,j,scribble))
			return true;
	}
	
	return false;
}

void Segmentator::SegmentOne(int scribble) 
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
	double c_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_C, 0);
	double l1_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L1, 0);
	double l2_norm = cvNorm(m_pFe->GetPrincipalChannels(), 0, CV_L2, 0);
	printf("PChannels matrix c norm = %lf\n", c_norm);
	printf("PChannels matrix l1 norm = %lf\n", l1_norm);
	printf("PChannels matrix l2 norm = %lf\n", l2_norm);
	cvConvertScale(m_pFe->GetPrincipalChannels(), pChannels, 0.02);

	CvMat * f_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	CvMat * b_mask = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	
	cvSetZero( m_Segmentations[scribble] );
	getMask(m_Segmentations[scribble], b_mask,1);
	
	
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
	GraphHandler::calc_beta(m_pImg->height, m_pImg->width, m_pFe->GetColorChannels());
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
#ifdef NEW_GMM	
#else
		f_gmm->GetAllProbabilities(pChannels, conf_map_fg);
		b_gmm->GetAllProbabilities(pChannels, conf_map_bg);
#endif
		
		// Set weights
		numeric_limits<double> limits;
		for (int i=0; i<m_pImg->height; i++)
		{
			for (int j=0; j<m_pImg->width; j++)
			{
				if (IsInScribble(i,j,scribble))
				{//inside the current scribble
					cvmSet(Fu,i,j,limits.infinity());
					cvmSet(Bu,i,j,0);
				
				}
				
				else if (IsInScribble(i,j))	
				{//inside one of the other scribbles
					cvmSet(Fu,i,j,0);
					cvmSet(Bu,i,j,limits.infinity());
				
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
		
		graph->do_MinCut(*m_Segmentations[scribble]);
		
		PrevFlow = CurFlow;
		CurFlow = graph->getFlow();
		//if (abs((CurFlow-PrevFlow)/CurFlow)<TOLLERANCE)
		//	break;

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

		// Update GMM
		getMask(m_Segmentations[scribble], f_mask,0);
		getMask(m_Segmentations[scribble], b_mask,1);
		
#ifdef NEW_GMM	
		f_gmm->NextStep(f_mask);
		b_gmm->NextStep(b_mask);
#else
		f_gmm->NextStep(pChannels, f_mask, CvEM::COV_MAT_DIAGONAL);
		b_gmm->NextStep(pChannels, b_mask, CvEM::COV_MAT_DIAGONAL);	
#endif
			
		delete graph;
	}
	
	// Save the last FG confidence map
	cvConvert(conf_map_fg, m_Probabilities[scribble]);

}

IplImage * Segmentator::GetSegmentedImage(int scribble)
{
	int step = m_pImg->widthStep;
	memcpy((uchar *)m_pSegImg->imageData,m_pImg->imageData, m_pImg->imageSize);

	uchar * pData  = (uchar *)m_pSegImg->imageData;

	for (int y = 0; y < m_pImg->height; y++)
	{
		for (int x = 0; x < m_pImg->width; x++)
		{
			int value = (int) cvmGet(this->m_Segmentations[scribble],y,x);

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

/*IplImage * Segmentator::GetSegmentedImage()
{
	int step = m_pImg->widthStep;
	cvCvtColor(m_pImg, m_pSegImg, CV_BGR2YCrCb);

	uchar * pData  = (uchar *)m_pSegImg->imageData;

	for (int y = 0; y < m_pImg->height; y++)
	{
		for (int x = 0; x < m_pImg->width; x++)
		{
			int value = (int) cvmGet(this->m_FinalSeg,y,x);
			
			pData[y*step+x*3+1] = m_scribbles[value].getU();
			pData[y*step+x*3+2] = m_scribbles[value].getV();
		}
	}

	return m_pSegImg;
}*/
// TODO:
/* Phase 2 - (Soft?) Colorization
 * 
 * Two options, I guess:
 * 
 * 1. 
 * 	a. Segmentation for each scribble
 * 	b. Assign color per pixel according to the segmentation
 * 	c. Deal with overlaps (Higher probability wins?)
 * 	d. Deal with pixels not in any segmentation
 * 	e. Seems like the logical choice (Easy to see why it would work)
 * 	f. Has some issues...... Doesn't sound smooth.
 *  g. We can enforce spatial locality. --> creativity!
 *	h. should we save the bg conf maps for our calculations? --> creativity!
 * 
 * 2.
 * 	a. Segmentation for each pixel _BUT_ only refer to the probabilities
 * 	b. Color each pixel according to a weighted average:
 * 
 * 		p(0) * c(0) + ... + p(n) * c(n)
 * 		-------------------------------
 * 		p(0) + p(1) + ... + P(n-1) + p(n)
 * 
 * 		Where p(i) the probability the pixel belongs to object i, c(i) the color of scribble (i)
 * 	c. Overlaps are handled
 * 	d. Pixels not in any segmentation are also handled (Would get some mixture of all the scribbles)
 *  e. A 'smoother' way to assign colors?
 * 	f. At the cost of ignoring the smoothness term (Which is ironic, really)?
 * 
 * 3.
 * 	Maybe somehow a mixture of the first two options?
 * 
 *
 * 4. We can use 'a graph cut approach' to our problem: for each overlapping segment we can build a graph:
 *		Two terminal nodes - for each of the two segments
 *		Each of the pixels in that segments have nodes, adjacent nodes has the 'regular' smoothness term
 *		And each node has an edge to each of the terminals weighed according to fg gmms
 *		An minimal cut for this graph corresponds each pixel to a single segment. 
 *		The biggest problem I can think of is it should work for when we have only two segments to decide from.
 * 5.
 * 	Be creative?
 * 
 * 6. Adjust GUI to the new multi-scribble-thingy - make sure loading/saving scribbles still works.
 */
 
void Segmentator::AssignColors()
{
	for (int n=0; n<m_nScribbles; n++) 
	{
		for (int i=0; i<m_FinalSeg->rows; i++)
			for (int j=0; j<m_FinalSeg->cols; j++) {
				int isInNSegment = cvmGet(m_Segmentations[n],i,j);
				int finalSegment = cvmGet(m_FinalSeg,i,j);
				//finalSegment = 0 --> no segment assigned!! I'm guessing thats wrong?
				
				if (!finalSegment && isInNSegment)//no overlapping
					finalSegment =  n ;
				else 
					if (finalSegment && isInNSegment)//overlapping
						finalSegment = decideSegment(i,j, n, finalSegment);
				cvmSet(m_FinalSeg,i,j,finalSegment);

			}
	}
}

int Segmentator::decideSegment(int i, int j, int seg1, int seg2) {


	double prob1 = cvmGet(m_Probabilities[seg1],i,j);
	double prob2 = cvmGet(m_Probabilities[seg2],i,j);

	if (prob1 > prob2)
		return seg1;
	else
		return seg2;

}
 
 
void Segmentator::Colorize()
{
  	// TODO:	
  	
  	// Segment();
  	// AssignColors();
  	// ?
}
