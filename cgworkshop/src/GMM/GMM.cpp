#include "GMM.h"

CGMM::CGMM()
{
	// FIXME
	m_nClusters = 5;
	m_nMaxIter = 1;
	m_nEpsilon = 0.05;		
}

void CGMM::Init(CvMat * pDataSet)
{
	Init(pDataSet, 0);	
}

void CGMM::Init(CvMat * pDataSet , CvMat * pActiveMask)
{
    // initialize model's parameters
    m_params.covs      = NULL;
    m_params.means     = NULL;
    m_params.weights   = NULL;
    m_params.probs     = NULL;
    m_params.nclusters = m_nClusters;
    m_params.cov_mat_type       = CvEM::COV_MAT_DIAGONAL;
    m_params.start_step         = CvEM::START_AUTO_STEP;
    m_params.term_crit.max_iter = m_nMaxIter;
    m_params.term_crit.epsilon  = m_nEpsilon;
    m_params.term_crit.type     = CV_TERMCRIT_ITER;
    
    m_model = new CvEM();
	m_model->train( pDataSet, pActiveMask, m_params); 	
}


void CGMM::NextStep(CvMat * pDataSet)
{
	NextStep(pDataSet, 0);	
}

void CGMM::NextStep(CvMat * pDataSet , CvMat * pActiveMask)
{
    // initialize model's parameters
    m_params.covs      = (const CvMat**)m_model->get_covs();
    m_params.means     	 = m_model->get_means();
    m_params.weights   = m_model->get_weights();
    m_params.probs     = NULL;
    m_params.start_step         = CvEM::START_E_STEP;
    
    
    // Switch to a new model, train it using the results of the old one
    CvEM * pNewModel = new CvEM();
	pNewModel->train( pDataSet, pActiveMask, m_params); 
	
	delete m_model;
	m_model = pNewModel;
}

float CGMM::GetProbability(CvMat * pFeatureVector)
{
	float * _prob = new float[m_nClusters]; // Take this!
	CvMat prob = cvMat( 1, m_nClusters, CV_32FC1, _prob );
	m_model->predict( pFeatureVector, &prob );
	
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
