#include "FeatureExtraction.h"
#include "cvgabor.h"

int CFeatureExtraction::GetGaborResponse()
{
	// TODO one gabor response per filter, 3 color channels?
	
	double Sigma = 2*PI;
	double F = sqrt(2.0);
	CvGabor *gabor1 = new CvGabor;
	gabor1->Init(PI/4, 3, Sigma, F);

	IplImage *reimg = cvCreateImage(cvSize(m_pSrcImg->width,m_pSrcImg->height), IPL_DEPTH_8U, 3);
	gabor1->conv_img(m_pSrcImg, reimg, CV_GABOR_MAG);
	cvNamedWindow("Real Response", 1);
	cvShowImage("Real Response",reimg);
	cvWaitKey(0);
	cvDestroyWindow("Real Response");
}

// TODO: Would fail if m_nChannels != 3
int CFeatureExtraction::GetColorPCA(CvMat * pColorChannels[])
{
	// Get our 32F matrix (From the 32F image created previously)	
	CvMat * pMat = cvCreateMat( m_nWidth*m_nHeight, 3 , CV_32F );
	memcpy(pMat->data.fl, (float*)m_pSrcImgFloat->imageData, m_pSrcImgFloat->imageSize);

	// Create our result matrices
	CvMat* avg = cvCreateMat( 1, 3, CV_32F );
	CvMat* eigenVectors = cvCreateMat( 3, 3, CV_32F );
	CvMat* eigenValues = cvCreateMat( 3, 1, CV_32F );	
	
	// Actual PCA calculation
	cvCalcPCA(pMat, avg, eigenValues, eigenVectors, CV_PCA_DATA_AS_ROW ); 
	
	// Useful debugging
	printf("Mean Vector: %f,%f,%f\n", cvmGet(avg, 0,0), cvmGet(avg, 0,1), cvmGet(avg, 0,2));
	printf("Eigen Values: %f,%f,%f\n", cvmGet(eigenValues, 0,0), cvmGet(eigenValues, 1,0), cvmGet(eigenValues, 2,0));
	printf("Eigen Vector 1: %f, %f, %f\n", cvmGet(eigenVectors, 0,0), cvmGet(eigenVectors, 0,1), cvmGet(eigenVectors, 0,2));
	printf("Eigen Vector 2: %f, %f, %f\n", cvmGet(eigenVectors, 1,0), cvmGet(eigenVectors, 1,1), cvmGet(eigenVectors, 1,2));
	printf("Eigen Vector 3: %f, %f, %f\n", cvmGet(eigenVectors, 2,0), cvmGet(eigenVectors, 2,1), cvmGet(eigenVectors, 2,2));
	
	// Transform to the new basis
	CvMat * pTransMat = cvCreateMat( m_nWidth*m_nHeight, 3 , CV_32F );
	cvMatMul(pMat, eigenVectors, pTransMat);

	// TODO: Normalize each channel by itself?		
	// Normalize the matrix (0..255)
	cvNormalize(pTransMat, pTransMat, 0, 255, CV_MINMAX);
	
	// Store each of the 3 p-channels in a matrix
	float val;
	for (int k=0;k<m_nChannels;k++)
	{
		for (int i=0;i<m_nHeight;i++)
		{
			for (int j=0;j<m_nWidth;j++)
			{
					val = pTransMat->data.fl[i*m_nWidth*3 + j*m_nChannels + k];
					pColorChannels[k]->data.ptr[i*m_nWidth+j] = (unsigned char) val;
			}
		}
	}
	
	// Save each channel to a bitmap, just for fun.
	char filename[255];
	IplImage * pImg = cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);
	char * tempData = pImg->imageData;
	for (int i=0;i<m_nChannels;i++)
	{
		sprintf(filename, "output/chan%d.bmp", i+1);
		pImg->imageData = (char *) pColorChannels[i]->data.ptr;

		printf("Saving pchannel %d to: %s\n",i+1, filename);
		if (!cvSaveImage(filename,pImg)) 
			printf("Could not save: %s\n",filename);
	}
	pImg->imageData = tempData;
	cvReleaseImage(&pImg);
	
	// Useful releasing
	cvReleaseMat(&pMat);
	cvReleaseMat(&avg);
	cvReleaseMat(&eigenVectors);
	cvReleaseMat(&eigenValues);
	cvReleaseMat(&pTransMat);
	return 0;	
}

CFeatureExtraction::CFeatureExtraction(char * file)
{
	m_pFile = file;
	
	// Load the input image
	m_pSrcImg = cvLoadImage(file,1);
	
	// Extract parameters
	m_nChannels = m_pSrcImg->nChannels;
	m_nWidth = m_pSrcImg->width;
	m_nHeight = m_pSrcImg->height; 	
	
	// Scale to a 32bit float image (needed for later stages)
	m_pSrcImgFloat = cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_32F,3);
	cvConvertScale(m_pSrcImg,m_pSrcImgFloat,1.0,0);
}

int CFeatureExtraction::Run()
{
	CvMat * pColorChannels[3];
	for (int i=0;i<3;i++)
		pColorChannels[i] = cvCreateMat( m_nWidth , m_nHeight , CV_8U );
		
	GetColorPCA(pColorChannels);

//	GetGaborResponse();

	for (int i=0;i<3;i++)
		cvReleaseMat(&pColorChannels[i]);


	
	return 0;
}
