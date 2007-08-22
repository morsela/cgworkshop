#ifndef _H_SEGMENTATOR_H_
#define _H_SEGMENTATOR_H_

#include <cv.h>
#include <highgui.h>

#include "fe/FeatureExtraction.h"
#include "GUI/TypeDefs.h"

#define MAX_ITER 150
#define TOLLERANCE 0.0000
#define BACKGROUND -1

#define ONE_SEG_PER_PIXEL_METHOD 0
#define AVG_COLOR_METHOD 1

class Segmentator 
{
public:
	Segmentator(IplImage * Img, ScribbleVector &scribbles, int nScribbles);
	virtual ~Segmentator();

public:

	void Colorize();
	void Segment();
	CvMat * getSegmentation(int scribble) {return m_Segmentations[scribble];}

	IplImage * GetSegmentedImage(int scribble);
	IplImage * GetSegmentedImage();

protected:
	
	void SegmentOne(int scribble);
	void CalcAverage(CvMat * Bg, CvMat * Fg, int scribble);
	
	void getMask(CvMat * segmentation, CvMat * mask, int isBackground);

	bool IsInScribble(int i, int j, int scribble);
	bool IsInScribble(int i, int j);
	void AssignColors();
	int decideSegment(int i,int j, int seg1, int seg2);
	
	void RecolorPixel(uchar * pData, int y, int x, CvScalar * pColor);
	
	void CountSegments();
	
	void AssignColor(int i, int j, CvScalar * color, int method);
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
	CvMat * m_FinalSeg;
	CvMat ** m_Probabilities;
	
};

#endif	//_H_SEGMENTATOR_H_
