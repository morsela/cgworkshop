#ifndef _H_GUI_H_
#define _H_GUI_H_

#include <cv.h>
#include <vector>

#include "Point.h"
#include <GL/glut.h>

class CScribble
{
public:

	CScribble(CPoint &startPoint, CPoint &endPoint)
	{
		m_startPoint	= startPoint;
		m_endPoint		= endPoint;
	}

	void Draw() const
	{
		glColor3f( 0.75,1.0,0.75 );
		glVertex3f( m_startPoint.x, m_startPoint.y, -0.2);
		glColor3f( 0.5,1.0,0.5 );
		glVertex3f(m_endPoint.x,  m_endPoint.y, -0.2);
	}
	
	CPoint m_startPoint;
	CPoint m_endPoint;

protected:

	unsigned int	m_color;
};

class CGUI
{
public:
	
	CGUI() {}
	virtual ~CGUI() {}

	int Setup(char * pszImagePath);

	static CGUI * GetInstance() { static CGUI inst; return &inst; }

	void LoadTextures();

public:

	int GetWidth() const { return m_nWindowWidth; }
	
	int GetHeight() const { return m_nWindowHeight; }

	void SetWindowSize(int x, int y) { m_nWindowWidth = x; m_nWindowHeight = y; }

	const IplImage * const GetImage() { return m_pImg; }

public:

	void MouseMove(int x, int y);
	
	void MouseAction(int button, int state, int x, int y);

	void KeysAction(unsigned char key, int x, int y );

	void Reshape(int x , int y);

	void Render();

protected:
	
	IplImage *		m_pImg;

	int				m_nWindowWidth;

	int				m_nWindowHeight;

	// the bounding box for the opengl orthogonal projection
	float 			m_orthoBBox[4];

	unsigned int	m_textures[1];

	// The vectors in object space coordinates (for rendering)
std::vector< CScribble >  m_scribbles;

// the vectors in image space coordinates (for saving)
std::vector< int >		leftVecs;
	

};

#endif	//_H_GUI_H_