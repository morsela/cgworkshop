#include "GMM.h"
#define MY_PI 3.141592

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
	
	CvMat * temp1, * temp2, * temp3, * temp4;

	temp1 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	temp2 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	temp3 = cvCreateMat( m_nDims, 1, CV_32FC1 );
	temp4 = cvCreateMat( 1, 1, CV_32FC1 );
	CvMat * mean = cvCreateMat( 1, m_nDims, CV_32FC1 );
	
	float * pData = pDataSet->data.fl;
	
	cvSetZero(pProbs);
	
	int k;
	for (k=0;k<m_nClusters;k++)
	{
		if (cvmGet(pWeights, 0,k) == 0)
			continue;
			
		cvGetRow(pMeans, mean, k);			
		CvMat * covInv = pCovsInv[k];
		
		double term1 = 	(pWeights->data.fl[k]/sqrt(pDet[k]));
		
		for (i=0;i<pDataSet->rows;i++)
		{
			cvInitMatHeader(&vector, 1, pDataSet->cols, CV_32FC1, &pData[i*pDataSet->cols]);
			cvSub(&vector, mean, temp1);
			cvTranspose(temp1, temp3);
			cvMatMul(temp1, covInv, temp2);	
			
			cvMatMul(temp2, temp3, temp4);	
			
			double hez = -0.5 * temp4->data.fl[0];
			double pi = exp(hez)*term1;
			
			pProbs->data.fl[i] += pi;
		}
	}

	//printf("Prob matrix norm=%lf\n", cvNorm(pProbs,0,CV_C));
	//cvConvertScale(pProbs, pProbs, 1./cvNorm(pProbs,0,CV_C), 0);
	//cvNormalize(pProbs, pProbs, 0.01, 1, CV_MINMAX);
	
	cvReleaseMat(&temp1);
	cvReleaseMat(&temp2);
	cvReleaseMat(&temp3);
	cvReleaseMat(&temp4);
}
