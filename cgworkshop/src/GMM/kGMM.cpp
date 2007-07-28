#include <math.h>
#include "kGMM.h"

#define SMALL_EPS 0.01
#define MY_PI 3.141592

CkGMM::CkGMM(int clusters, int max_iterations, double accuracy)
{
	printf("CkGMM::CkGMM in\n");
	
	m_nClusters = clusters;
	m_nMaxIterations = max_iterations;
	m_nAccuracy = accuracy;
	
	printf("CkGMM::CkGMM out\n");
}

void CkGMM::Init(CvMat * pDataSet, CvMat * pActiveMask)
{
	int i;
	
	printf("CkGMM::Init in\n");
	
	m_pAllSamplesMat = pDataSet;
	
	m_nDims = m_pAllSamplesMat->cols;
	m_nSamplesCount = m_pAllSamplesMat->rows;
	m_nActiveCount = (int) cvNorm(pActiveMask, 0, CV_L1, 0);
	printf("CkGMM::Init We have %d samples, each of dimension %d\n", m_nSamplesCount, m_nDims);
	printf("CkGMM::Init We have %d active samples\n", m_nActiveCount);
	
	m_pProbabilityMat = cvCreateMat( m_nSamplesCount, 1, CV_32F );
	m_pLabelMat = cvCreateMat(m_nSamplesCount, 1, CV_8UC1 );

	m_pInvCovMats = new CvMat*[m_nClusters];
    for (i=0;i<m_nClusters;i++)
    	m_pInvCovMats[i] = cvCreateMat( m_nDims, m_nDims, CV_32FC1 );

	m_pMeanVecs = new CvMat*[m_nClusters];
    for (i=0;i<m_nClusters;i++)
    	m_pMeanVecs[i] = cvCreateMat( 1, m_nDims, CV_32FC1 );

    m_pWeightVec   =  cvCreateMat(m_nClusters, 1, CV_32FC1 );
    m_pDetVec = new double[m_nClusters];
    
    
    m_ppCompData = new (CvMat*)[m_nSamplesCount];
    for (i=0;i<m_nSamplesCount;i++)
    	m_ppCompData[i] = new CvMat;

	InitClusters(pActiveMask);
	NextStep(pActiveMask);
	
	printf("CkGMM::Init out\n");	
}

void CkGMM::NextStep(CvMat * pActiveMask)
{
	printf("CkGMM::NextStep In\n");	
	
	ComputeParams(pActiveMask);
	ComputerProbabilities(pActiveMask);
			
	printf("CkGMM::NextStep out\n");	
}

void CkGMM::InitClusters(CvMat * pActiveMask)
{
	int i;
	printf("CkGMM::InitClusters in\n");
	
	// Create matrix of active samples only
	CvMat * pActiveSamples = cvCreateMat( m_nActiveCount, m_nDims, CV_32FC1 );
	
	// Populate
	int step = pActiveSamples->step;
	unsigned char * pActive = pActiveSamples->data.ptr;
	unsigned char * pAll = m_pAllSamplesMat->data.ptr;
	
	int count = 0;
	for (i=0;i<m_nSamplesCount;i++)
	{
		//printf("Testing sample %d\n", i);
		if (pActiveMask->data.ptr[i])
		{
			//printf("Active sample, copying to index %d\n", count);
			memcpy(	&pActive[count*step], &pAll[i*step], step);
			count++;
		}
	}
	printf("CkGMM::InitClusters Expected %d samples, got %d\n", m_nActiveCount, count);

	// Find initial clusters using kmeans
	CvMat * pActiveLabels = cvCreateMat( m_nActiveCount, 1, CV_32SC1 );
	cvKMeans2(pActiveSamples, m_nClusters, pActiveLabels,  
		cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 20, 0.001 ));
		
	// Generate labels for all samples
	count = 0;
	for (i=0;i<m_nSamplesCount;i++)
	{
		int cluster = -1;
		if (pActiveMask->data.ptr[i])
		{
			cluster = pActiveLabels->data.i[count];
			count++;
		}
		
		m_pLabelMat->data.ptr[i] = cluster;
	}
	printf("CkGMM::InitClusters Expected %d samples, got %d\n", m_nActiveCount, count);
	
	// Free
	cvReleaseMat(&pActiveSamples);
	cvReleaseMat(&pActiveLabels);
	
	printf("CkGMM::InitClusters out\n");	
}

void CkGMM::ComputeParams(CvMat * pActiveMask)
{
	int i,j;
	
	printf("CkGMM::ComputeParams in\n");

	// Calculate model parameters
	CvMat * pCovMat = cvCreateMat( m_nDims, m_nDims, CV_32FC1 );
	for (i=0;i<m_nClusters;i++)
	{
		printf("CkGMM::ComputeParams Cluster %d\n",i);
		
		int count = 0;
		// Find all features of component i
		for (j=0;j<m_nSamplesCount;j++)
		{
			
			if (pActiveMask->data.ptr[j] && (m_pLabelMat->data.ptr[j] == i))
			{
				cvInitMatHeader(m_ppCompData[count], 1, m_nDims, CV_32FC1, &m_pAllSamplesMat->data.fl[j*m_nDims]);
				count++;
			}
		}
		
		if (count == 0)
			continue;
		
		printf("Count(%d) = %d\n", i, count);
		// Calc covariance matrix, mean
		cvCalcCovarMatrix( (const void **)m_ppCompData, count, pCovMat, m_pMeanVecs[i], CV_COVAR_SCALE + CV_COVAR_NORMAL );
		
		printf("MeanxMean[%d]=%lf\n", i, cvDotProduct( m_pMeanVecs[i], m_pMeanVecs[i]));
		/*
		cvSetZero( m_pMeanVecs[i] );
		
		count = 0;
		for (j=0;j<m_nSamplesCount;j++)
		{
			
			if (pActiveMask->data.ptr[j] && (m_pLabelMat->data.ptr[j] == i))
			{
				cvInitMatHeader(m_ppCompData[count], 1, m_nDims, CV_32FC1, &m_pAllSamplesMat->data.fl[j*m_nDims]);
				//printf("Dot=%lf\n", cvDotProduct(m_ppCompData[count],m_ppCompData[count]));
				cvAdd( m_ppCompData[count], m_pMeanVecs[i], m_pMeanVecs[i]);
				count++;
			}
		}
				
		cvScale(m_pMeanVecs[i], m_pMeanVecs[i], 1./count);
		printf("MeanxMean[%d]=%lf\n", i, cvDotProduct( m_pMeanVecs[i], m_pMeanVecs[i]));
		*/
		// Calc weight
		m_pWeightVec->data.fl[i] = (double)count/m_nSamplesCount;
		
		// Make covariance matrix non-singular
	    double det = cvDet(pCovMat);
	    	
		while (det < SMALL_EPS) 
		{
			for (j=0;j<m_nDims;j++)
				cvmSet(pCovMat, j, j, cvmGet(pCovMat, j,j)+SMALL_EPS);
				det = cvDet(pCovMat);
			} 	    	
			
		m_pDetVec[i] = cvInvert(pCovMat, m_pInvCovMats[i]);	
	    printf("Det(%d)=%lf\n", i, m_pDetVec[i]);
	    printf("DetInv(%d)=%lf\n", i, cvDet(m_pInvCovMats[i]));
	    printf("Weight(%d)=%lf\n", i, m_pWeightVec->data.fl[i]);
	}

	cvReleaseMat(&pCovMat);

	printf("CkGMM::ComputeParams out\n");			
}

void CkGMM::ComputerProbabilities(CvMat * pActiveMask)
{
	int i,j;
	printf("CkGMM::ComputerProbabilities in\n");

	CvMat * temp1 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	CvMat * temp2 = cvCreateMat( 1, m_nDims, CV_32FC1 );
	CvMat vector;

	for (i=0;i<m_nSamplesCount;i++)
	{
		cvGetRow(m_pAllSamplesMat,&vector, i);

		int cluster = 0;
		double max = 0;
		double min = 10000000;
		double prob = 0;
		{
			int i;
			for (i=0;i<m_nClusters;i++)
			{
				// Ignore empty clusters
				if (m_pWeightVec->data.fl[i] == 0)
					continue;
		
				//printf("Mean*Mean=%lf\n", cvDotProduct(m_pMeanVecs[i], m_pMeanVecs[i]));
				
				// Calculate the exponent
				cvSub(&vector, m_pMeanVecs[i], temp1);		
					
				//printf("Diff*Diff=%lf\n", cvDotProduct(temp1, temp1));
				
				cvMatMul(temp1, m_pInvCovMats[i], temp2);
				
				//printf("temp2*temp2=%lf\n", cvDotProduct(temp2, temp2));
				
				double expo = -0.5 * cvDotProduct(temp1, temp2);
				//printf("Exp[%d]=%lf\n", i, expo);
				//printf("Det[%d]=%lf\n", i, m_pDetVec[i]);
				
				//-probability vector belongs to this cluster
				double p_i = exp(expo) * pow(m_pDetVec[i],-0.5);
				//printf("Prob[%d]=%lf\n",i, p_i);
				
				// Track closest cluster
				if (p_i > max)
				{
					max = p_i;
					cluster = i;	
				}
				
				// Calc total probability
				prob += m_pWeightVec->data.fl[i]*p_i;

				
			}
		}
		//printf("Prob[%d]=%lf\n", i, max);
		m_pLabelMat->data.ptr[i] = cluster;
		
		m_pProbabilityMat->data.fl[i] = prob;
	}

	// Free
	cvReleaseMat(&temp1);
	cvReleaseMat(&temp2);

	printf("CkGMM::ComputerProbabilities out\n");		
}

/*
	printf("CkGMM:: in\n");

	printf("CkGMM:: out\n");	
*/	

