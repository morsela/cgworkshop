#include "fe/FeatureExtraction.h"
#include "GMM/GMM.h"
#include "GraphHandler.h"
#include "GUI/TypeDefs.h"
#include <cv.h>
#include <algorithm>
using namespace std;
#define MAX_ITER 6


class Segmentator {



public:
	Segmentator::Segmentator(IplImage * Img, CFeatureExtraction *fe, ScribbleVector scribbles);
	virtual ~Segmentator();


public:
	void Segment();

protected:
	void doGraphCuts();
	void getMask(CvMat *mask, int isBackground);


private:
	std::vector<CPointInt> m_points;
	CFeatureExtraction *m_pFe;
	IplImage * m_pImg;
	CvMat * m_Segmentation;
	
};