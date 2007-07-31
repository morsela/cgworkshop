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
	Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector &scribbles, int nScribbles);
	virtual ~Segmentator();

public:

	void Segment();
	CvMat * getSegmentation() {return m_Segmentation;}

	IplImage * GetSegmentedImage();

protected:
	
	void getMask(CvMat * mask, int nScribble);
	void getDoubleMask(CvMat * mask, int nScribble1, int nScribble2);
	void UpdateSegmentation(CvMat * pPartialSeg, int nScribble1, int nScribble2, CvMat * pDoubleMask);
	

private:

	ScribbleVector	m_scribbles;
	int				m_nScribbles;
	
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	IplImage* m_pSegImg;
	CvMat * m_Segmentation;
	
};

#endif	//_H_SEGMENTATOR_H_
