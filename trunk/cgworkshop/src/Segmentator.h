#ifndef _H_SEGMENTATOR_H_
#define _H_SEGMENTATOR_H_

#include <cv.h>
#include <highgui.h>

#include "fe/FeatureExtraction.h"
#include "GUI/TypeDefs.h"

#define MAX_ITER 5
#define TOLLERANCE 0.05

class Segmentator 
{
public:
	Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector &scribbles, int nScribbles);
	virtual ~Segmentator();

public:

	void Segment();
	CvMat * getSegmentation(int scribble) {return m_Segmentations[scribble];}

	IplImage * GetSegmentedImage(int scribble);

protected:
	
	void SegmentOne(int scribble);
	
	void getMask(CvMat * segmentation, CvMat * mask, int isBackground);

	bool IsInScribble(int i, int j, int scribble);
	bool IsInScribble(int i, int j);

private:

	ScribbleVector	m_scribbles;
	int				m_nScribbles;
	
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	IplImage* m_pSegImg;
	CvMat ** m_Segmentations;
	
};

#endif	//_H_SEGMENTATOR_H_
