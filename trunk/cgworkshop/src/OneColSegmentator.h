#pragma once
#include "SegmentatorBase.h"

class OneColSegmentator : public SegmentatorBase
{

public:
	OneColSegmentator(IplImage * Img, ScribbleVector &scribbles, int nScribbles) :
		SegmentatorBase(Img, scribbles, nScribbles){};
	
	~OneColSegmentator(void){};

public:
	virtual void AssignColor(int i, int j, CvScalar * color);

private:
	int decideSegment(int i, int j, int seg1, int seg2);
	
};
