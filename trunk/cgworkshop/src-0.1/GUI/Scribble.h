#ifndef _H_SCRIBBLE_H_
#define _H_SCRIBBLE_H_

#include <cv.h>
#include <vector>

#include <GL/glut.h>

#include <fstream>

#include "Point.h"

class CScribble 
{
public:

	CScribble();
	
	CScribble(int nID);
	virtual ~CScribble();

public:

	void AddObjectPoint(CPointFloat & point);

	void AddObjectPoint(float x, float y);

	void AddImagePoint(CPointInt & point);

	void AddImagePoint(int x, int y);

	bool Load(std::ifstream &ifs);

	bool Save(char * pszFilename);

	void Draw() const;

public: 

	std::vector<CPointInt> GetScribblePoints()	{ return m_pImagePoints;}

	int	GetScribbleSize() const				{ return m_nPoints; }

	int GetID() const						{ return m_nID;};

protected:

	std::vector<CPointFloat>	m_pObjectPoints;

	std::vector<CPointInt>		m_pImagePoints;

	int							m_nPoints;

	int							m_nID;


};

#endif	//_H_SCRIBBLE_H_
