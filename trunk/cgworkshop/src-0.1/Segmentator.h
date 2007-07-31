#ifndef _H_SEGMENTATOR_H_
#define _H_SEGMENTATOR_H_

#include <cv.h>
#include <highgui.h>

#include "fe/FeatureExtraction.h"
#include "GUI/TypeDefs.h"

#define MAX_ITER 10
#define TOLLERANCE 0.05

class Segmentator 
{
public:
	Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector scribbles);
	virtual ~Segmentator();

public:

	void Segment();
	CvMat * getSegmentation() {return m_Segmentation;}

	IplImage * GetSegmentedImage();

protected:
	
	void getMask(CvMat *mask, int isBackground);

private:

	std::vector<CPointInt> m_points;
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	IplImage* m_pSegImg;
	CvMat * m_Segmentation;
	
};

#endif	//_H_SEGMENTATOR_H_
