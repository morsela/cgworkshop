#pragma once
#include "SegmentatorBase.h"

class AEColSegmentator : public SegmentatorBase
{

public:
	AEColSegmentator(IplImage * Img, ScribbleVector &scribbles, int nScribbles) :
		SegmentatorBase(Img, scribbles, nScribbles){};
	
	~AEColSegmentator(void){};


protected:

	virtual void Segment();
	void SegmentAll();
	virtual void AssignColor(int i, int j, CvScalar * color);
	void CreateFinalImage();

};
