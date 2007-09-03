#include "OneColSegmentator.h"

void OneColSegmentator::AssignColor(int i, int j, CvScalar *color) 
{
	CvScalar scrYUV;
	
	int finalSegment = BACKGROUND;
	
	for (int n=0; n<m_nScribbles; n++) 
	{	
		int isInNSegment = cvmGet(m_Segmentations[n],i,j);	
		int isBg = (cvmGet(m_SegmentCount, i,j) == 0);
		
		// Keep one sided segmentation working...
		if (m_nScribbles == 1)
			isBg = 0;
				
		if (finalSegment==BACKGROUND && (isInNSegment || isBg)) //no overlapping
			finalSegment =  n;
		else if (isInNSegment || isBg)//overlapping
			finalSegment = decideSegment(i,j, n, finalSegment);	
	}
	
	if (finalSegment == BACKGROUND)
	{
		color->val[0] = 0;
		color->val[1] = 0;
		color->val[2] = 0;
		
		RGB2YUV(color, &scrYUV);
		memcpy(color->val, scrYUV.val, sizeof(CvScalar));		
	}
	else
	{
		RGB2YUV(m_scribbles[finalSegment].GetColor(), &scrYUV);
		memcpy(color->val, scrYUV.val, sizeof(CvScalar));
		
		double prob = cvmGet(m_Probabilities[finalSegment],i,j);	
		
		//color->val[0] *= prob;
		//color->val[1] *= prob;
		//color->val[2] *= prob;		
	}
}


int OneColSegmentator::decideSegment(int i, int j, int seg1, int seg2)
{


	double prob1 = exp(-cvmGet(m_Probabilities[seg1],i,j));
	double prob2 = exp(-cvmGet(m_Probabilities[seg2],i,j));
	double bgprob1 = exp(-cvmGet(m_BGProbabilities[seg1],i,j));
	double bgprob2 = exp(-cvmGet(m_BGProbabilities[seg2],i,j));
	
	// Disabled for now, -log and all

	if (bgprob1 < prob1 && bgprob2 < prob2) //No segment should 'win'.
			return BACKGROUND; 
	
	if (prob1*bgprob2 > prob2*bgprob1)
		return seg1;
	else
		return seg2;

}
 

