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

	void Reset();

	void AddObjectPoint(CPointFloat & point);

	void AddObjectPoint(float x, float y);

	void AddImagePoint(CPointInt & point);

	void AddImagePoint(int x, int y);

	bool Load(std::ifstream &ifs);

	bool Save(char * pszFilename);

	void Draw() const;

	bool Find(CPointInt p);
	
	void SetColor(const CvScalar * color);

public: 

	CPointInt & operator[](int i);

	void SetID(int id)							{ m_nID = id; }

	int	GetScribbleSize() const				{ return m_nPoints; }

	int GetID() const						{ return m_nID;};

	bool IsValid()	const					{ return (m_nPoints > 0); }
	
	CvScalar * GetColor()					{ return &m_color; }


protected:

	std::vector<CPointFloat>	m_pObjectPoints;

	std::vector<CPointInt>		m_pImagePoints;

	int							m_nPoints;

	int							m_nID;
	
	CvScalar					m_color;


};

#endif	//_H_SCRIBBLE_H_
