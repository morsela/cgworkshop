#include "kGMM.h"

CkGMM::CkGMM()
{
	m_nClusters = 5;
}

void CkGMM::Init(CvMat * pDataSet)
{
	int i,j;
	int nSamples = pDataSet->rows;
	int nFeatures = pDataSet->cols;
	
	float * _pIdx = new float[nSamples];
	CvMat IdxMat =  cvMat( nSamples, 1, CV_32SC1, _pIdx );
	
	// Find initial clusters using kmeans
	cvKMeans2( pDataSet , m_nClusters , &IdxMat, cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 0.01 ));
	
	// Create matrices
	pWeightVec = cvCreateMat( m_nClusters, 1 , CV_32F );
	
	pMeansVecs = new (CvMat*)[m_nClusters];
	for (i=0;i<m_nClusters;i++)
		pMeansVecs[i] = cvCreateMat( 1, nFeatures , CV_32F );
			
	pCovMats = new (CvMat*)[m_nClusters];
	for (i=0;i<m_nClusters;i++)
		pCovMats[i] = cvCreateMat( nFeatures, nFeatures , CV_32F );
	
	// Calculate initial model parameters
	CvMat ** pComponentData = new (CvMat*)[nSamples];
	for (i=0;i<m_nClusters;i++)
	{
		int count = 0;
		// Find all features of component i
		for (j=0;j<nSamples;j++)
		{
			
			if (IdxMat.data.i[j] == i)
			{
				cvInitMatHeader(pComponentData[count], 1, nFeatures, CV_32FC1, &pDataSet->data.fl[j*nFeatures]);
				count++;
			}
		}
		
		// Calc covariance matrix, mean
		cvCalcCovarMatrix( (const void **)pComponentData, count, pCovMats[i], pMeansVecs[i], CV_COVAR_SCALE | CV_COVAR_NORMAL );
		
		// Calc weight
		pWeightVec->data.fl[i] = count/nSamples;
	}
}


void CkGMM::NextStep(CvMat * pDataSet)
{

}

float CkGMM::GetProbability(CvMat * pFeatureVector)
{
}

void CkGMM::GetAllProbabilities(CvMat * pDataSet, CvMat * pProbs)
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
