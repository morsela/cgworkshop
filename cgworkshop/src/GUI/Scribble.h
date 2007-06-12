#pragma once
#include <cv.h>
#include <string>

using namespace std;


class Scribble {

public:
	Scribble();
	~Scribble();

public:
	void loadScribble(string filename, CvMat *pchannels);

	CvMat * getScribble() { return m_Scribbles;};

	int getId() const {return m_id;};





private:
	CvMat * m_Scribbles;
	int m_id;


};
