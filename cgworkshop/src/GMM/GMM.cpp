#include "GMM.h"
#define MY_PI 3.141592

#define EXPIREMENTAL_PROBABILITY_CALCULATION

#define SMALL_EPS 0.1

CGMM::CGMM():m_model(NULL)
{
	// FIXME
	m_nClusters = 5;
	m_nMaxIter = 1;
	m_nEpsilon = 0.01f;		
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

	pCovsInv = new CvMat*[m_nClusters];
    for (i=0;i<m_nClusters;i++)
    	pCovsInv[i] = cvCreateMat( dims, dims, CV_32FC1 );

    pMeans     	 = cvCreateMat( m_nClusters, dims, CV_32FC1 );
    pWeights   =  cvCreateMat( 1, m_nClusters, CV_32FC1 );
    pProbs     	 = cvCreateMat( 1, m_nClusters, CV_32FC1 );
    
    
    pDet = new double[m_nClusters];
    
    m_params.covs = (const CvMat **) pCovs;
    m_params.means = pMeans;
    m_params.weights = pWeights;
    
	m_params.probs     = NULL;
	
    m_params.nclusters = m_nClusters;
    m_params.cov_mat_type       = covType;
    m_params.start_step         = CvEM::START_AUTO_STEP;
    m_params.term_crit.max_iter = m_nMaxIter;
    m_params.term_crit.epsilon  = m_nEpsilon;
    m_params.term_crit.type     = CV_TERMCRIT_ITER+CV_TERMCRIT_EPS;
    
    //m_model = new CvEM(pDataSet,pActiveMask,m_params,0);

	printf("GMM: Active pixels=%f\n", cvNorm(pActiveMask,0,CV_L1,0));

	m_model = new CvEM();

	OneStep(pDataSet, pActiveMask);	
}


void CGMM::OneStep(CvMat * pDataSet, CvMat * pActiveMask)
{
	int i;
	
	m_model->train( pDataSet, pActiveMask, m_params); 
	
	cvConvert(m_model->get_weights(), pWeights);
	
    const CvMat ** cov_mats = m_model->get_covs();
    for (i=0;i<m_nClusters;i++)
    {
    	cvConvert(cov_mats[i], pCovs[i]);
    	
    	// Make sure Det is positive
    	int j;
    	for (j=0;j<m_nDims;j++)
			cvmSet(pCovs[i], j, j, cvmGet(pCovs[i], j,j)+SMALL_EPS);

		pDet[i] = cvInvert(pCovs[i], pCovsInv[i]);	
    	printf("Det(%d)=%lf\n", i, cvDet(pCovs[i]));
    }

	cvConvert(m_model->get_means(), pMeans);	
}

void CGMM::NextStep(CvMat * pDataSet , CvMat * pActiveMask, int covType)
{
    m_params.start_step         = CvEM::START_E_STEP;
    m_params.cov_mat_type       = covType;

	printf("GMM: Active pixels=%f\n", cvNorm(pActiveMask,0,CV_L1,0));
	
	OneStep(pDataSet, pActiveMask);	
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
		double max = 0;
		prob = 0;
		{
			int i;
				int j;
			for (i=0;i<m_nClusters;i++)
			{
				if (cvmGet(pWeights, 0,i) == 0)
					continue;

				cvGetRow(pMeans, mean, i);			
				cvSub(&vector, mean, temp1);			
				cvMatMul(temp1, pCovsInv[i], temp2);
				
				double hez = -0.5 * cvDotProduct(temp1, temp2);
				//printf("Hez[%d]=%lf\n", i, hez);
				
				double pi = exp(hez)*(pWeights->data.fl[i]/sqrt(pDet[i]));
				prob += pi;

				
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
