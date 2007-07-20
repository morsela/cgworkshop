#include "GMM.h"

CGMM::CGMM():m_model(NULL)
{
	// FIXME
	m_nClusters = 5;
	m_nMaxIter = 1;
	m_nEpsilon = 0.05f;		
}

void CGMM::Init(CvMat * pDataSet)
{
//	Init(pDataSet, 0);	
}

void CGMM::Init(CvMat * pDataSet , CvMat * pActiveMask, int covType)
{
    // initialize model's parameters
    int i;
	int dims = pDataSet->cols;

	pCovs = new CvMat*[m_nClusters];
    for (i=0;i<m_nClusters;i++)
    	pCovs[i] = cvCreateMat( dims, dims, CV_64FC1 );

    pMeans     	 = cvCreateMat( m_nClusters, dims, CV_64FC1 );
    pWeights   =  cvCreateMat( 1, m_nClusters, CV_64FC1 );
    pProbs     	 = cvCreateMat( 1, m_nClusters, CV_64FC1 );
    
    
    m_params.covs = (const CvMat **) pCovs;
    m_params.means = pMeans;
    m_params.weights = pWeights;
    
	m_params.probs     = NULL;
	
    m_params.nclusters = m_nClusters;
    m_params.cov_mat_type       = covType;
    m_params.start_step         = CvEM::START_AUTO_STEP;
    m_params.term_crit.max_iter = m_nMaxIter;
    m_params.term_crit.epsilon  = m_nEpsilon;
    m_params.term_crit.type     = CV_TERMCRIT_ITER;
    
    //m_model = new CvEM(pDataSet,pActiveMask,m_params,0);

	m_model = new CvEM();
	m_model->train( pDataSet, pActiveMask, m_params); 
	cvConvert(m_model->get_weights(), pWeights);
}


void CGMM::NextStep(CvMat * pDataSet)
{
//	NextStep(pDataSet, 0);	
}

void CGMM::NextStep(CvMat * pDataSet , CvMat * pActiveMask, int covType)
{
	int i;

    // initialize model's parameters
    const CvMat ** cov_mats = m_model->get_covs();
    for (i=0;i<m_nClusters;i++)
    	cvConvert(cov_mats[i], pCovs[i]);

	cvConvert(m_model->get_means(), pMeans);
//    m_params.start_step         = CvEM::START_E_STEP;
    m_params.cov_mat_type       = covType;

    // Switch to a new model, train it using the results of the old one
    //delete m_model;
   	//m_model = new CvEM();

	m_model->train( pDataSet, pActiveMask, m_params); 
	cvConvert(m_model->get_weights(), pWeights);
}

double CGMM::GetProbability(CvMat * pFeatureVector)
{
	m_model->predict( pFeatureVector, pProbs );
	return cvDotProduct(pWeights,pProbs);
}

void CGMM::GetAllProbabilities(CvMat * pDataSet, CvMat * pProbs)
{
	int i;
	CvMat vector;
	
	float * pData = pDataSet->data.fl;
	for (i=0;i<pDataSet->rows;i++)
	{
		cvInitMatHeader(&vector, 1, pDataSet->cols, CV_32FC1, &pData[i*pDataSet->cols]);
		double prob = GetProbability(&vector);

		pProbs->data.fl[i] = prob;
	}
}
