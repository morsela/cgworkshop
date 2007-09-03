#include "AvgColSegmentator.h"


void AvgColSegmentator::AssignColor(int i, int j, CvScalar *color) 
{
	int segCount = cvmGet(m_SegmentCount, i,j);
	
	if (0 && segCount == 1)
	{
		for (int n=0; n<m_nScribbles; n++) 
		{
			if (cvmGet(m_Segmentations[n],i,j))
			{
					RGB2YUV(m_scribbles[n].GetColor(), color);
					return;
			}
		}
	}

	// Assigned to multiple segmentations
	// Average out colors of all assigned scribbles
	CvScalar scrYUV;
	double val[3];
	{
		
		double probCount = 0;
		
		val[0] = 0;
		val[1] = 0;
		val[2] = 0;	
		
		double maxProb = 0;
		double minProb = 1000;
		double sumProb = 0;
		for (int n=0; n<m_nScribbles; n++) 
		{
			double prob = (cvmGet(m_Probabilities[n],i,j));
			if (prob < minProb)
				minProb = prob;			
			if (prob > maxProb)
				maxProb = prob;
				
			sumProb += prob;
		}
			
		for (int n=0; n<m_nScribbles; n++) 
		{

			double prob = maxProb-(cvmGet(m_Probabilities[n],i,j))+minProb;
			//double prob = sumProb-(cvmGet(m_Probabilities[n],i,j));
			
			// Give more weight to selected scribbles
			if (cvmGet(m_Segmentations[n],i,j))
				prob *= 10;
			
			//printf("Prob[%d]=%lf\n", n,prob);
			RGB2YUV(m_scribbles[n].GetColor(), &scrYUV);
			
			val[0] += scrYUV.val[0] * prob;
			val[1] += scrYUV.val[1] * prob;
			val[2] += scrYUV.val[2] * prob;
				
			probCount += prob;
		}
			
		val[0] /= probCount;
		val[1] /= probCount;
		val[2] /= probCount;
		
		color->val[0] = val[0];
		color->val[1] = val[1];
		color->val[2] = val[2];
	
	}	
}
