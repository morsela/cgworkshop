#ifndef _H_SCRIBBLE_H_
#define _H_SCRIBBLE_H_

#include <cv.h>

class CScribble {

public:
	CScribble();
	virtual ~CScribble();

public:

	void Load(char * filename);

	CPoint * GetScribblePoints() { return m_pPoints;}

	int	GetScribbleSize() const { return m_nPoints; }

	int GetID() const {return m_nID;};





private:

	CPoint *	m_pPoints;

	int			m_nPoints;

	int			m_nID;


};

#endif	//_H_SCRIBBLE_H_