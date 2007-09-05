#ifndef _H_SEGMENTATOR_H_
#define _H_SEGMENTATOR_H_

#include <cv.h>
#include <highgui.h>

#include "fe/FeatureExtraction.h"
#include "GUI/TypeDefs.h"

#define MAX_ITER 10
#define TOLLERANCE 0.005
#define BACKGROUND -1


class SegmentatorBase
{
public:
	SegmentatorBase(IplImage * Img, ScribbleVector &scribbles, int nScribbles);
	virtual ~SegmentatorBase();

public:

	void Colorize();

	virtual IplImage * GetSegmentedImage(int scribble);
	IplImage * GetSegmentedImage();

protected:
	

	void getMask(CvMat * mask, int label);
	void PrintStatus(CvMat ** masks);
	
	void RecolorPixel(uchar * pData, int y, int x, CvScalar * pColor);
	void RGB2YUV(CvScalar * pRGB, CvScalar * pYUV);

	void SegmentOne(int scribble);

	virtual void Segment();
	void AssignColors();
	virtual void AssignColor(int i, int j, CvScalar * color) = 0;

	
	void CountSegments();
	
	

protected:

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
