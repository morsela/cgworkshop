#ifndef __GMM_H__
#define __GMM_H__

#include "ml.h"
#include "highgui.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class CGMM 
{
public:
	
	CGMM();
	~CGMM();

	// Initializes the model for given data set
	void Init(CvMat * pDataSet);
	
	void Init(CvMat * pDataSet, CvMat * pActiveMask);

	// Runs a single EM iteration using the given data set
	void OneStep(CvMat * pDataSet, CvMat * pActiveMask);
	
	void NextStep(CvMat * pDataSet, CvMat * pActiveMask);
	
	
	// Returns the probability for the given feature vector
	double GetProbability(CvMat * pFeatureVector);
	
	// Returns a matrix that contains, for each feature vector in the data set,
	// the probability it belongs to either of our two classes
	// TODO: probably return this in a different format, figure out which
	void GetAllProbabilities(CvMat * pDataSet, CvMat * pProbs);
	
private:

	CvEM * m_model;
	int m_nClusters;
	int m_nMaxIter;
	int m_nMaxWeight;
	int m_nDims;
	float m_nEpsilon;
	CvEMParams m_params;
	
	double * pDet;
	CvMat ** pCovs;
	CvMat ** pCovsInv;
	CvMat * pMeans;
	CvMat * pWeights;
	CvMat * pProbs;
};


/*
 * Code would be along the lines of:
 * 

 * 
 * Evaluate(CvMat * pDataSet);
 *
	int m_nClusters = 5;
	int m_nMaxIter = 10;
	float m_nEpsilon = 0.1;
	
	CvEMParams params;
	CvEM em_model;

    // initialize model's parameters
    params.covs      = NULL;
    params.means     = NULL;
    params.weights   = NULL;
    params.probs     = NULL;
    params.nclusters = m_nClusters;
    params.cov_mat_type       = CvEM::COV_MAT_SPHERICAL;
    params.start_step         = CvEM::START_AUTO_STEP;
    params.term_crit.max_iter = m_nMaxIter;
    params.term_crit.epsilon  = m_nEpsilon;
    params.term_crit.type     = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
    
	em_model.train( pDataSet, 0, params); 
 *
 * float GetProbability(CvMat * pFeatureVector);
 *
	float _prob[m_nClusters]; // Take this!
	CvMat prob = cvMat( 1, m_nClusters, CV_32FC1, _prob );
	em_model.predict( pFeatureVector, &prob );
	// return the max value in prob
	
 *
 * void GetAllProbabilities(CvMat * pDataSet, CvMat * pProbs);
 * 

	// For each feature vector in the set,
	// call GetProbability
	// return in a fancy format

 */

#endif // __GMM_H__
