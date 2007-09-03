#pragma once
#include "SegmentatorBase.h"

class AvgColSegmentator : public SegmentatorBase
{

public:
	AvgColSegmentator(IplImage * Img, ScribbleVector &scribbles, int nScribbles) :
		SegmentatorBase(Img, scribbles, nScribbles){};
	
	~AvgColSegmentator(void){};

public:
	virtual void AssignColor(int i, int j, CvScalar * color);


};
