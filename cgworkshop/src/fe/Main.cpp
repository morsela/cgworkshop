#include "FeatureExtraction.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cv.h>
#include <cxcore.h>

int main(int argc, char ** argv) {
	if (argc < 2 && 0)
	{
		printf("Usage:\n");
		printf("	Image : fe <image>\n");
		return -1;
	}
	printf("Doing feature extraction for %s\n", argv[1]);
	
	CFeatureExtraction * fe;


	printf("Loading image %s\n", argv[1]);
	IplImage *pSrcImg = cvLoadImage(argv[1],1);
	
	fe = new CFeatureExtraction(pSrcImg);
	fe->Run();
	
	return 0;
	
}
