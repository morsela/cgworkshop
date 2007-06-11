#ifndef __FEATURE_EXTRACTION_H__
#define __FEATURE_EXTRACTION_H__

#include <stdio.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define COLOR_CHANNEL_NUM	3
#define TEXTURE_CHANNEL_NUM	3

class CFeatureExtraction 
{
	public:
		CFeatureExtraction(char * file);
		virtual ~CFeatureExtraction();

		bool Run();
		
	public:

		bool GetColorChannels(CvMat * pColorChannels[]);
		bool GetTextureChannels(CvMat * pTextureChannels[]);
		
	protected:
		
		bool GetGaborResponse(CvMat * pGaborMat);
		bool GetGaborResponse(IplImage *pGrayImg, IplImage *pResImg, float orientation, float freq, float sx, float sy);
		
		void CalcHistogram(IplImage * pImg, CvMat * pHistogram);
		
		bool GetChannels(CvMat * pMergedMat, CvMat * pChannels[], int nTotalChans, int nExtractChans);
		bool DoPCA(CvMat * pMat, CvMat * pResultMat, int nSize, int nExpectedSize); 

	protected:
		
		IplImage * 	m_pSrcImg;
		IplImage * 	m_pSrcImgFloat;
		
		int			m_nWidth;
		int			m_nHeight;
		int			m_nChannels;
		char *		m_pFile;

		CvMat *		m_pColorChannels[COLOR_CHANNEL_NUM];
		CvMat *		m_pTextureChannels[TEXTURE_CHANNEL_NUM];
		
};

#endif // __FEATURE_EXTRACTION_H__
