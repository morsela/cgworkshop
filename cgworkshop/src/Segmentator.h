#ifndef _H_SEGMENTATOR_H_
#define _H_SEGMENTATOR_H_

#include <cv.h>
#include <highgui.h>

#include "fe/FeatureExtraction.h"
#include "GUI/TypeDefs.h"

#define MAX_ITER 5
#define TOLLERANCE 0.005
#define BACKGROUND -1

#define ONE_SEG_PER_PIXEL_METHOD 0
#define AVG_COLOR_METHOD 1
#define AE_COLOR_METHOD 2

class Segmentator 
{
public:
	Segmentator(IplImage * Img, ScribbleVector &scribbles, int nScribbles);
	virtual ~Segmentator();

public:

	void Colorize();
	void Segment();

	IplImage * GetSegmentedImage(int scribble);
	IplImage * GetSegmentedImage();

protected:
	
	void CreateFinalImage();
	void SegmentAll();
	void getMask(CvMat * mask, int label);
	void PrintStatus(CvMat ** masks);
	
	void RecolorPixel(uchar * pData, int y, int x, CvScalar * pColor);
	void RGB2YUV(CvScalar * pRGB, CvScalar * pYUV);

	void SegmentOne(int scribble);
	void CalcAverage(CvMat * Bg, CvMat * Fg, int scribble);


	void AssignColors();
	int decideSegment(int i,int j, int seg1, int seg2);
	
	void CountSegments();
	
	void AssignColor(int i, int j, CvScalar * color, int method);
	void AssignColorAE(int i, int j, CvScalar * color);
	void AssignColorOneSeg(int i, int j, CvScalar * color);
	void AssignColorAvgColor(int i, int j, CvScalar * color);


private:

	ScribbleVector	m_scribbles;
	int				m_nScribbles;
	
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	IplImage* m_pSegImg;
	IplImage* m_pTempSegImg;

	CvMat ** m_Segmentations;
	CvMat * m_SegmentCount;
	CvMat ** m_Probabilities;
	CvMat ** m_BGProbabilities;

	CvMat * m_pLabels;
	CvMat * m_pScribbles;
	
};

#endif	//_H_SEGMENTATOR_H_
