#include "GMM.h"

CGMM::CGMM()
{
	// FIXME
	m_nClusters = 5;
	m_nMaxIter = 1;
	m_nEpsilon = 0.05;		
}

void CGMM::Evaluate(CvMat * pDataSet)
{
	Evaluate(pDataSet, 0);	
}

void CGMM::Evaluate(CvMat * pDataSet , CvMat * pActiveMask)
{
	CvEMParams params;
	
    // initialize model's parameters
    params.covs      = NULL;
    params.means     = NULL;
    params.weights   = NULL;
    params.probs     = NULL;
    params.nclusters = m_nClusters;
    params.cov_mat_type       = CvEM::COV_MAT_DIAGONAL;
    params.start_step         = CvEM::START_AUTO_STEP;
    params.term_crit.max_iter = m_nMaxIter;
    params.term_crit.epsilon  = m_nEpsilon;
    params.term_crit.type     = CV_TERMCRIT_ITER;
    
	m_model.train( pDataSet, pActiveMask, params); 	
}

float CGMM::GetProbability(CvMat * pFeatureVector)
{
	float * _prob = new float[m_nClusters]; // Take this!
	CvMat prob = cvMat( 1, m_nClusters, CV_32FC1, _prob );
	m_model.predict( pFeatureVector, &prob );
	
	int i=0;
	float max = _prob[i];
	for (i=1;i<m_nClusters;i++)
		if (_prob[i] > max)
			max = _prob[i];
	
	return max;
}

void CGMM::GetAllProbabilities(CvMat * pDataSet, CvMat * pProbs)
{
	int i;
	CvMat vector;
	
	float * pData = pDataSet->data.fl;
	for (i=0;i<pDataSet->rows;i++)
	{
		cvInitMatHeader(&vector, 1, pDataSet->cols, CV_32FC1, &pData[i*pDataSet->cols]);
		float prob = GetProbability(&vector);
		
		pProbs->data.fl[i] = prob;
	}
}
