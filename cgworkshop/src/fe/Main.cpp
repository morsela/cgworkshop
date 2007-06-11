#include "FeatureExtraction.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char ** argv) {
	if (argc < 2 && 0)
	{
		printf("Usage:\n");
		printf("	Image : fe <image>\n");
		return -1;
	}
	printf("Doing feature extraction for %s\n", argv[1]);
	
	CFeatureExtraction * fe;

	fe = new CFeatureExtraction(argv[1]);
	fe->Run();
	
	return 0;
	
}

