#ifndef __KGMM_H__
#define __KGMM_H__

#include "ml.h"
#include "highgui.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class CkGMM 
{
public:
	// Constructors
	CkGMM(int clusters);
	CkGMM(int clusters, int max_iterations);
	CkGMM(int clusters, double accuracy);
	CkGMM(int clusters, int max_iterations, double accuracy);
	
	
	// Initializes the model for given data set
	void Init(CvMat * pDataSet, CvMat * pActiveMask);
	
	// Performs a single GMM iterations on the active samples given
	void NextStep(CvMat * pActiveMask);
	
	// Return probabilities for all samples
	CvMat * GetProbabilities() { return m_pProbabilityMat; }
	
private:
	void ComputeParams(CvMat * pActiveMask);
	void ComputerProbabilities(CvMat * pActiveMask);
	void InitClusters(CvMat * pActiveMask);
	void ComputeMeans(CvMat * pActiveMask);
	void ComputeCovar(CvMat * pActiveMask);
	
private:

	// General parameters
	int 	m_nClusters;
	int		m_nMaxIterations;
	double		m_nAccuracy;
	
	// Sample releted vars
	CvMat * m_pAllSamplesMat;
	int 	m_nSamplesCount;
	int 	m_nActiveCount;
	int		m_nDims;

	// Sample output
	CvMat *	m_pProbabilityMat;
	CvMat * m_pLabelMat;
	
	// GMM parameters
	CvMat ** m_pMeanVecs;
	CvMat ** m_pInvCovMats;
	double * m_pDetVec;
	CvMat *	 m_pWeightVec;
	
	CvMat ** m_ppCompData;

};


#endif // __KGMM_H__
