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
		bool Run();
		
	private:
		bool GetColorChannels(CvMat * pColorChannels[]);
		bool GetTextureChannels(CvMat * pTextureChannels[]);
		
		bool GetGaborResponse(CvMat * pGaborMat);
		bool GetGaborResponse(IplImage *pGrayImg, IplImage *pResImg, float orientation, float freq, float sx, float sy);
		
		void CalcHistogram(IplImage * pImg, CvMat * pHistogram);
		
		bool GetChannels(CvMat * pMergedMat, CvMat * pChannels[], int nTotalChans, int nExtractChans);
		bool DoPCA(CvMat * pMat, CvMat * pResultMat, int nSize); 

	protected:
		
		IplImage * 	m_pSrcImg;
		IplImage * 	m_pSrcImgFloat;
		
		int			m_nWidth;
		int			m_nHeight;
		int			m_nChannels;
		char *		m_pFile;
		
};

#endif // __FEATURE_EXTRACTION_H__
