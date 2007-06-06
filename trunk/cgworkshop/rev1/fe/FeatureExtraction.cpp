#include "FeatureExtraction.h"
#include "cvgabor.h"
#include <math.h>

inline double round2( double d )
{
	return floor( d + 0.5 );
}


// TODO:
// Combine 30 Histogram values with 24 Gabor values
// PCA these.

static void displayImage(char * title, IplImage * pImg)
{
	cvNamedWindow(title, 1);
	cvShowImage(title,pImg);
	cvWaitKey(0);
	cvDestroyWindow(title);		
}

//////////////////////////////////////////////////////////////////////////////////////

// TODO: Make sure working......
// Image seems alright, but not perfect
void CFeatureExtraction::CalcHistogram(IplImage * pImg, CvMat * pHistogram)
{
	int nBins = 10;
	
	int step = pImg->widthStep;
	int channels = m_nChannels;
	int w = m_nWidth;
	int h = m_nHeight;
	uchar * pData  = (uchar *)pImg->imageData;
	
	for (int y=0; y<h; y++)
	{
		for (int x=0;x<w; x++)
		{
			for (int k=0;k<channels;k++)
			{
				// Get appropriate bin for current pixel
				uchar val = pData[y*step+x*channels+k];;
				uchar bin = val*nBins/255;
	
				// Go over a 5x5 patch, increase appropriate bin by 1
				for (int j=y-2;j<=y+2;j++)
				{
	
					if (j<0 || j>=h)
	                	continue;

	                for (int i=x-2;i<=x+2;i++)
	                {
	                    if (i<0 || i>=w)
	                            continue;
						
						//int row = j*w+x;
						pHistogram->data.fl[j*step*10+x*channels*10 +k*nBins+bin]+=1;
	                }
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

bool CFeatureExtraction::GetGaborResponse(IplImage *pGrayImg, IplImage *pResImg, float orientation, float freq, float sx, float sy)
{
	// Create the filter
	CvGabor *pGabor = new CvGabor(orientation, freq, sx, sy);
	
	//pGabor->show(CV_GABOR_REAL);
	
	// Convolution
	pGabor->conv_img(pGrayImg, pResImg, CV_GABOR_MAG);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

bool CFeatureExtraction::GetGaborResponse(CvMat * pGaborMat)
{
	float* pMatPos;
	int idx = 0;
		
	// Convert our image to grayscale (Gabor doesn't care about colors! I hope?)	
	IplImage *pGrayImg = cvCreateImage(cvSize(m_pSrcImg->width,m_pSrcImg->height), IPL_DEPTH_8U, 1);
	cvCvtColor(m_pSrcImg,pGrayImg,CV_BGR2GRAY);

	// The output image
	IplImage *reimg = cvCreateImage(cvSize(pGrayImg->width,pGrayImg->height), IPL_DEPTH_8U, 1);

	double freq = 0.4;
	int freq_steps = 4;
	int ori_count = 6;
	double ori_space = PI/ori_count;
	
	int i,j;
	for (i=0;i<freq_steps;i++)	
	{
    	double bw = (2 * freq) / 3;
    	double sx = round2(0.5 / PI / pow(bw,2));
    	double sy = round2(0.5 * log(2.0) / pow(PI,2.0) / pow(freq,2.0) / (pow(tan(ori_space / 2),2.0)));	
    		
		for (j=0;j<ori_count;j++)
		{	
			double ori = j*ori_space;
			GetGaborResponse(pGrayImg, reimg, ori, freq, sx, sy);
			
			// This being a test and all, display the image
			// displayImage(title, reimg);
			
			// Concat the new vector to the result matrix
			int k;
			pMatPos = (float *) pGaborMat->data.fl;
			char * pResData = (char *) reimg->imageData;
			for (k=0;k<reimg->width*reimg->height;k++)
			{
				pMatPos[idx] = (float) pResData[0];
				pMatPos += 24;
				pResData++;
							
			}

			idx++;
		}
		freq /= 2;	
	}
	// Release
	cvReleaseImage(&reimg);
	cvReleaseImage(&pGrayImg);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

// TODO: Would fail if m_nChannels != 3
// RGB to LAB
bool CFeatureExtraction::GetColorChannels(CvMat * pColorChannels[])
{
	int nSize = 3;
	
	// Convert to LAB color space
	IplImage *pLabImg = cvCreateImage(cvSize(m_pSrcImg->width,m_pSrcImg->height), IPL_DEPTH_32F, nSize);
	cvCvtColor(m_pSrcImgFloat,pLabImg,CV_BGR2Lab);	

	// Get our 32F matrix (From the 32F image created previously)	
	CvMat * pMat = cvCreateMat( m_nWidth*m_nHeight, nSize , CV_32F );
	memcpy(pMat->data.fl, (float*)pLabImg->imageData, pLabImg->imageSize);

	// This matrix would hold the values represented in the new basis we've found
	CvMat * pResultMat = cvCreateMat( m_nWidth*m_nHeight, nSize , CV_32F );
	// Actual calculation
	DoPCA(pMat, pResultMat, nSize);
	// Extracting the 3 primary channels
	GetChannels(pResultMat, pColorChannels, nSize, 3);
	
	// Useful releasing
	cvReleaseMat(&pMat);
	cvReleaseMat(&pResultMat);
	
	return true;	
}

//////////////////////////////////////////////////////////////////////////////////////

bool CFeatureExtraction::GetTextureChannels(CvMat * pTextureChannels[])
{
	int i;

	int gaborSize = 24;
	int histSize = 30;
	int vectorSize = gaborSize+histSize;
	
	// Calc the full histogram vectors
	CvMat * pHistMat = cvCreateMat( m_nWidth*m_nHeight, histSize , CV_32F );
	CalcHistogram(m_pSrcImg, pHistMat);
	// Do we need to normalize?
	cvNormalize(pHistMat, pHistMat, 0, 255, CV_MINMAX);

	CvMat * pGaborMat = cvCreateMat (m_nWidth * m_nHeight, gaborSize, CV_32F);
	GetGaborResponse(pGaborMat);
	// Do we need to normalize?
	cvNormalize(pGaborMat, pGaborMat, 0, 255, CV_MINMAX);

	// Our merged matrix
	CvMat * pTextureMat = cvCreateMat( m_nWidth*m_nHeight, vectorSize , CV_32F );

	// Go over row by row, concat the gabor and histogram matrices
	float * pMatData = (float *) pTextureMat->data.fl;
	float * pHistData = (float *)  pHistMat->data.fl;
	float * pGaborData = (float *)  pGaborMat->data.fl;
	
	int gaborStep = pGaborMat->step;
	int histStep = pHistMat->step;
	
	for (i=0;i<m_nWidth * m_nHeight;i++)
	{
		
		memcpy(pMatData, pGaborData, gaborStep);
		pMatData+=gaborSize;
		pGaborData+=gaborSize;		
		
		memcpy(pMatData, pHistData, histStep);
		pMatData+=histSize;
		pHistData+=histSize;
	}
	

	// This matrix would hold the values represented in the new basis we've found
	CvMat * pResultMat = cvCreateMat( m_nWidth*m_nHeight, vectorSize , CV_32F );
	// Actual calculation
	DoPCA(pTextureMat, pResultMat, vectorSize);
	// Extracting the 3 primary channels
	GetChannels(pResultMat, pTextureChannels, vectorSize, 3);

	cvReleaseMat(&pHistMat);
	cvReleaseMat(&pGaborMat);
	cvReleaseMat(&pTextureMat);
	cvReleaseMat(&pResultMat);

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////

bool CFeatureExtraction::DoPCA(CvMat * pMat, CvMat * pResultMat, int nSize)
{
	printf("CFeatureExtraction::DoPCA\n");
	
	// Create our result matrices
	CvMat* pMeanVec = cvCreateMat( 1, nSize, CV_32F );
	CvMat* pEigenVecs = cvCreateMat( nSize, nSize, CV_32F );
	CvMat* pEigenVals = cvCreateMat( nSize, 1, CV_32F );	
	
	// Actual PCA calculation
	cvCalcPCA(pMat, pMeanVec, pEigenVals, pEigenVecs, CV_PCA_DATA_AS_ROW ); 
	/*
	// Useful debugging
	printf("Mean Vector: %f,%f,%f\n", cvmGet(pMeanVec, 0,0), cvmGet(pMeanVec, 0,1), cvmGet(pMeanVec, 0,2));
	printf("Eigen Values: %f,%f,%f\n", cvmGet(pEigenVals, 0,0), cvmGet(pEigenVals, 1,0), cvmGet(pEigenVals, 2,0));
	printf("Eigen Vector 1: %f, %f, %f\n", cvmGet(pEigenVecs, 0,0), cvmGet(pEigenVecs, 0,1), cvmGet(pEigenVecs, 0,2));
	printf("Eigen Vector 2: %f, %f, %f\n", cvmGet(pEigenVecs, 1,0), cvmGet(pEigenVecs, 1,1), cvmGet(pEigenVecs, 1,2));
	printf("Eigen Vector 3: %f, %f, %f\n", cvmGet(pEigenVecs, 2,0), cvmGet(pEigenVecs, 2,1), cvmGet(pEigenVecs, 2,2));
	*/

	// Transpose or not to transpose?
	// I assume eigenVectors contains one eigen vector per row, not column	
	CvMat* pTransposedVecs = cvCreateMat( nSize, nSize, CV_32F );
	cvTranspose(pEigenVecs, pTransposedVecs);
	
	cvMatMul(pMat, pTransposedVecs, pResultMat);
	
	// Release
	cvReleaseMat(&pTransposedVecs); 
	cvReleaseMat(&pMeanVec); 
	cvReleaseMat(&pEigenVecs); 
	cvReleaseMat(&pEigenVals); 
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

// Only nExtractChans=3 is supported!!!
bool CFeatureExtraction::GetChannels(CvMat * pMergedMat, CvMat * pChannels[], int nTotalChans, int nExtractChans)
{
	static int s_nChannel = 1;

	printf("CFeatureExtraction::GetChannels\n");
	
	// Store each of the 3 p-channels in a matrix
	float val;
	for (int k=0;k<nExtractChans;k++)
	{
		for (int i=0;i<m_nHeight;i++)
		{
			for (int j=0;j<m_nWidth;j++)
			{
				val = pMergedMat->data.fl[(i*m_nWidth+j)*nTotalChans + k];
				((float*)pChannels[k]->data.ptr)[i*m_nWidth+j] = val;
			}
		}
		// Normalize to 0..1
		cvNormalize(pChannels[k],pChannels[k], 0, 1, CV_MINMAX);
	}

	
	// Save each channel to a bitmap, just for fun.
	char filename[255];
	IplImage * pImg = cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);
	CvMat * pTempMat = cvCreateMat( m_nWidth, m_nHeight, CV_8U );
	char * tempData = pImg->imageData;
	for (int i=0;i<m_nChannels;i++)
	{
		sprintf(filename, "chan%d.bmp", s_nChannel);
		// Convert to unsigned char, 0..255
		cvConvertScale(pChannels[i],pTempMat,255,0);
		// Use the image header on our data
		pImg->imageData = (char *) pTempMat->data.ptr;
		
		// TODO: Remove this, only a test
		displayImage(filename, pImg);
		
		printf("Saving pchannel %d to: %s\n",s_nChannel, filename);
		if (!cvSaveImage(filename,pImg)) 
			printf("Could not save: %s\n",filename);
			
		s_nChannel++;	
	}
	pImg->imageData = tempData;
	cvReleaseMat(&pTempMat);
	cvReleaseImage(&pImg);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

CFeatureExtraction::CFeatureExtraction(char * file)
{	
	// Load the input image
	m_pSrcImg = cvLoadImage(file,1);
	if (m_pSrcImg == NULL)
		return;

	m_pFile = file;
	
	// Extract parameters
	m_nChannels = m_pSrcImg->nChannels;
	m_nWidth = m_pSrcImg->width;
	m_nHeight = m_pSrcImg->height; 	
	
	// Scale to a 32bit float image (needed for later stages)
	m_pSrcImgFloat = cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_32F,3);
	cvConvertScale(m_pSrcImg,m_pSrcImgFloat,1.0,0);
}

//////////////////////////////////////////////////////////////////////////////////////

bool CFeatureExtraction::Run()
{
	int i;

	CvMat * pColorChannels[3];
	for (i=0;i<3;i++)
		pColorChannels[i] = cvCreateMat( m_nWidth , m_nHeight , CV_32F );

	GetColorChannels(pColorChannels);

	CvMat * pTextureChannels[3];
	for (i = 0; i < 3; i++)
		pTextureChannels[i] = cvCreateMat(m_nWidth, m_nHeight, CV_32F);

	GetTextureChannels(pTextureChannels);

	for (i=0;i<3;i++)
		cvReleaseMat(&pColorChannels[i]);
	for (i=0;i<3;i++)
		cvReleaseMat(&pTextureChannels[i]);

	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
