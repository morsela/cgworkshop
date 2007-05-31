#ifndef __FEATURE_EXTRACTION_H__
#define __FEATURE_EXTRACTION_H__

#include <stdio.h>
#include <cv.h>
#include <cvaux.h>
#include <cxcore.h>
#include <highgui.h>

class CFeatureExtraction 
{
	public:
		CFeatureExtraction(char * file);
		int Run();
		
	private:
		int GetColorPCA(CvMat * pColorChannels[]);
		int GetGaborResponse();
		int GetGaborResponse(IplImage *pGrayImg, IplImage *pResImg, double orientation, int scale);
		void GetHistogram(CvMat * pHistVectors[]);
		void CalcHistogram(IplImage * pImg, CvMat * pHistogram);
		
	private:
		
		IplImage * 	m_pSrcImg;
		IplImage * 	m_pSrcImgFloat;
		
		int			m_nWidth;
		int			m_nHeight;
		int			m_nChannels;
		char *		m_pFile;
		
};

#endif // __FEATURE_EXTRACTION_H__
