#include "GMM.h"
#define MY_PI 3.141592

#define EXPIREMENTAL_PROBABILITY_CALCULATION

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
m_nDims = dims;
	pCovs = new CvMat*[m_nClusters];
    for (i=0;i<m_nClusters;i++)
    	pCovs[i] = cvCreateMat( dims, dims, CV_32FC1 );

    pMeans     	 = cvCreateMat( m_nClusters, dims, CV_32FC1 );
    pWeights   =  cvCreateMat( 1, m_nClusters, CV_32FC1 );
    pProbs     	 = cvCreateMat( 1, m_nClusters, CV_32FC1 );
    
    
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

	printf("GMM: Active pixels=%f\n", cvNorm(pActiveMask,0,CV_L1,0));

	m_model = new CvEM();
	m_model->train( pDataSet, pActiveMask, m_params); 
	
	cvConvert(m_model->get_weights(), pWeights);
	
    const CvMat ** cov_mats = m_model->get_covs();
    for (i=0;i<m_nClusters;i++)
    {
    	cvConvert(cov_mats[i], pCovs[i]);
    	printf("Det(%d)=%lf\n", i, cvDet(pCovs[i]));
    }

	cvConvert(m_model->get_means(), pMeans);	
}


void CGMM::NextStep(CvMat * pDataSet)
{
//	NextStep(pDataSet, 0);	
}

void CGMM::NextStep(CvMat * pDataSet , CvMat * pActiveMask, int covType)
{
	int i;

    m_params.start_step         = CvEM::START_E_STEP;
    m_params.cov_mat_type       = covType;

	printf("GMM: Active pixels=%f\n", cvNorm(pActiveMask,0,CV_L1,0));
	m_model->train( pDataSet, pActiveMask, m_params); 
	
	cvConvert(m_model->get_weights(), pWeights);
	const CvMat ** cov_mats = m_model->get_covs();
    for (i=0;i<m_nClusters;i++)
	{
    	cvConvert(cov_mats[i], pCovs[i]);
    	printf("Det(%d)=%lf\n", i, cvDet(pCovs[i]));
	}
	cvConvert(m_model->get_means(), pMeans);
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
	
	CvMat * covInv, * temp1, * temp2;
	covInv = cvCreateMat( m_nDims, m_nDims, CV_32FC1 );
	temp1 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	temp2 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	
	CvMat * mean = cvCreateMat( 1, m_nDims, CV_32FC1 );
	
	float * pData = pDataSet->data.fl;
	for (i=0;i<pDataSet->rows;i++)
	{
		cvInitMatHeader(&vector, 1, pDataSet->cols, CV_32FC1, &pData[i*pDataSet->cols]);
		double prob = GetProbability(&vector);
	
#ifdef EXPIREMENTAL_PROBABILITY_CALCULATION
		prob = 0;
		{
			int i;
			for (i=0;i<m_nClusters;i++)
			{
				double det = cvInvert(pCovs[i], covInv);
				//printf("Cluster %d, det=%f\n", i, det);
				if (det == 0) det = 0.01;
				cvGetRow(pMeans, mean, i);
				
				cvSub(&vector, mean, temp1);
				//printf("covInv=(%d,%d), temp1=(%d,%d)\n", covInv->rows, covInv->cols, temp1->rows, temp1->cols);	
				cvMatMul(temp1, covInv, temp2);
				double hez = -0.5 * cvDotProduct(temp1, temp2);
				//printf("Hez=%f\n", hez);
				
				double pi = exp(hez)/sqrt(det) * pow(2*MY_PI,m_nDims/2);
				prob += pWeights->data.fl[i] * pi;
				//printf("Prob=%f\n", pi);
				
			}
		}
#endif

		pProbs->data.fl[i] = prob;
//		printf("prob=%f\n", prob);
	}
	
	cvReleaseMat(&covInv);
	cvReleaseMat(&temp1);
	cvReleaseMat(&temp2);
}
