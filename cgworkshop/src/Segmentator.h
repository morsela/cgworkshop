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

	void Colorize();
	void Segment();
	CvMat * getSegmentation(int scribble) {return m_Segmentations[scribble];}

	IplImage * GetSegmentedImage(int scribble);
	IplImage * GetSegmentedImage();

protected:
	
	void SegmentOne(int scribble);
	
	void getMask(CvMat * segmentation, CvMat * mask, int isBackground);

	bool IsInScribble(int i, int j, int scribble);
	bool IsInScribble(int i, int j);
	void AssignColors();
	int decideSegment(int i,int j, int seg1, int seg2);

private:

	ScribbleVector	m_scribbles;
	int				m_nScribbles;
	
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	IplImage* m_pSegImg;
	CvMat ** m_Segmentations;
	CvMat * m_FinalSeg;
	CvMat ** m_Probabilities;
	
};

#endif	//_H_SEGMENTATOR_H_
