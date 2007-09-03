#include "AEColSegmentator.h"

#include <time.h>
#include "GraphHandlerAE.h"
#include "GMM/GMM.h"


void AEColSegmentator::Segment()
{
	m_pFe = new CFeatureExtraction(m_pImg);
	m_pFe->Run();

	cvSet(m_pScribbles, cvScalar(-1), 0);
	for (int scribble=0;scribble<m_nScribbles;scribble++)
	{
		for (int i = 0; i < (int)(m_scribbles[scribble].GetScribbleSize());i++)
		{
			CPointInt pI = m_scribbles[scribble][i];
			int x = pI.x;
			int y = pI.y;

			cvmSet(m_pScribbles, y, x, scribble);
		}	
	}

	printf("Segment\n");
	
	if (m_nScribbles > 1)
		SegmentAll();
	else
	{
		for (int i=0;i<m_nScribbles;i++)
			SegmentOne(i);	
	}
	
	delete m_pFe;
}



// Alpha expansion segmentation

void AEColSegmentator::SegmentAll() 
{
	srand(time(NULL));
	
	printf("+SegmentAll\n");

	CvMat * pChannels = cvCreateMat(m_pFe->GetPrincipalChannels()->rows,m_pFe->GetPrincipalChannels()->cols,m_pFe->GetPrincipalChannels()->type);
	CvMat * pColorChannels = cvCreateMat(m_pFe->GetColorChannels()->rows,m_pFe->GetColorChannels()->cols,m_pFe->GetColorChannels()->type);
	cvConvertScale(m_pFe->GetPrincipalChannels(), pChannels, 1);
	cvConvertScale(m_pFe->GetColorChannels(), pColorChannels, 1);	

	printf("+Create matrices\n");
	CGMM ** pGmmArr = new CGMM*[m_nScribbles];
	CvMat ** pMasksArr = new CvMat*[m_nScribbles];
	CvMat ** pProbArr = new CvMat*[m_nScribbles];
	
	CvMat * pNewLabels = cvCreateMat(m_pImg->height,m_pImg->width, CV_32F );
	
	for (int i=0;i<m_nScribbles;i++)
		pGmmArr[i] = new CGMM();
	for (int i=0;i<m_nScribbles;i++)
		pMasksArr[i] = cvCreateMat( 1, pChannels->rows, CV_8UC1 );
	for (int i=0;i<m_nScribbles;i++)
		pProbArr[i] = cvCreateMat(m_pImg->height, m_pImg->width, CV_32F );

	printf("-Create matrices\n");
	
	printf("+Init sutff\n");
	
	for (int scribble=0;scribble<m_nScribbles;scribble++)
	{
		printf("+Init GMM scribble=%d\n", scribble);
		cvSetZero( pMasksArr[scribble] );
		
		printf("+Setting scribble mask scribble=%d\n", scribble);
		//get initial foreground mask
		for (int i = 0; i < (int)(m_scribbles[scribble].GetScribbleSize());i++)
		{
			CPointInt pI = m_scribbles[scribble][i];
			int x = pI.x;
			int y = pI.y;

			pMasksArr[scribble]->data.ptr[y*m_pImg->width+x]=1;
		}	
		printf("-Setting scribble mask scribble=%d\n", scribble);
		
		pGmmArr[scribble]->Init(pChannels, pMasksArr[scribble]);
		
		printf("-Init GMM scribble=%d\n", scribble);
	}
	
	double alpha = 40;
	CGraphHandlerAE * pGraphHandler = new CGraphHandlerAE(m_pImg->height, m_pImg->width, pColorChannels, alpha);
	
	printf("-Init sutff\n");
	
	printf("+Do segmentations\n");
	
	for (int n=0; n < MAX_ITER; n++)
	{
		printf("+Iteration %d\n",n+1);	
		
		printf("+Get probabilities\n");
		for (int i=0;i<m_nScribbles;i++)
			pGmmArr[i]->GetAllProbabilities(pChannels, pProbArr[i]);
		printf("-Get probabilities\n");
		
		printf("+Graph cut\n");
		int success = 0;
		
		int cycle = 0;
		
		// Start with any labeling
		cvSet(m_pLabels,cvScalarAll((double)(m_nScribbles-1)),0);

		double energy = pGraphHandler->CalcEnergy(m_pLabels, pProbArr);
		printf("Initial energy is %lf\n", energy);
		do
		{
			printf("+ Alpha expansion cycle %d\n",cycle+1);
			success = 0;
			for (int i=0;i<m_nScribbles;i++)
			{
				printf("+ Alpha expansion iteration (label=%d)\n",i);
				
				pGraphHandler->DoAlphaExpansion(i, m_pLabels, pNewLabels, pProbArr, m_pScribbles);
				double newEnergy = pGraphHandler->CalcEnergy(pNewLabels, pProbArr);
				printf("New energy is %lf\n", newEnergy);
				
				if (newEnergy < energy)
				{
					printf("Alpha expansion improved energy, keeping...\n");
					energy = newEnergy;
					success = 1;
					
					cvConvertScale(pNewLabels, m_pLabels, 1);
				}
				else
				{
					printf("Alpha expansion DID NOT improve energy, ignoring...\n");	
				}
				
				printf("- Alpha expansion iteration (label=%d)\n",i);
			}
			
			printf("- Alpha expansion cycle %d\n",cycle+1);
			
			cycle ++;
		} while (success);
		
		printf("-Graph cut\n");
		
		printf("+GMM Step\n");
		
		for (int scribble=0;scribble<m_nScribbles;scribble++)
		{
			getMask(pMasksArr[scribble], scribble);
			pGmmArr[scribble]->NextStep(pChannels, pMasksArr[scribble]);
		}

		printf("-GMM Step\n");		
		
		PrintStatus(pMasksArr);
		
		printf("-Iteration %d\n",n+1);	
	}
	
	
	printf("-Do segmentations\n");
	
	printf("-SegmentAll\n");	

	delete pGraphHandler;

	cvReleaseMat(&pChannels);
	cvReleaseMat(&pColorChannels);
	cvReleaseMat(&pNewLabels);
		
	for (int i=0;i<m_nScribbles;i++) {
		delete pGmmArr[i];
		cvReleaseMat(&pMasksArr[i] );
		cvReleaseMat(&pProbArr[i]);
	}
	delete [] pGmmArr;
	delete [] pMasksArr;
	delete [] pProbArr;
}


void AEColSegmentator::AssignColor(int i, int j, CvScalar * color)
{
	int label = (int) cvmGet(m_pLabels, i,j);
	
	RGB2YUV(m_scribbles[label].GetColor(), color);
}

void AEColSegmentator::CreateFinalImage()
{
	printf("+CreateFinalImage\n");
	cvCvtColor(m_pImg, m_pSegImg, CV_BGR2YCrCb);

	uchar * pData  = (uchar *)m_pSegImg->imageData;
	
	CvScalar color;
	for (int i=0; i<m_pLabels->rows; i++)
	{
		for (int j=0; j<m_pLabels->cols; j++) 
		{
			AssignColor(i,j,&color);
			RecolorPixel(pData, i,j, &color);
		}
	}

	cvCvtColor(m_pSegImg, m_pSegImg, CV_YCrCb2BGR);	
	printf("-CreateFinalImage\n");
}