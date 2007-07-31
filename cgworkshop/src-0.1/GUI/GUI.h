#ifndef _H_GUI_H_
#define _H_GUI_H_

#include <cv.h>

#include <GL/glut.h>

#include "TypeDefs.h"
#include "Loader.h"

class CGUI
{
public:
	
	//TODO: move to cpp. you are lazy!
	CGUI() { m_fScribbling = false; }
	virtual ~CGUI() {}

	int Setup(char * pszImagePath, char * pScribbleFile = NULL);

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
	
	void AddScribblePoints(int x, int y);

protected:
	
	IplImage *			m_pImg;

	int					m_nWindowWidth;

	int					m_nWindowHeight;

	// the bounding box for the opengl orthogonal projection
	float 				m_orthoBBox[4];

	unsigned int		m_textures[1];

	// The vectors in object space coordinates (for rendering)
	ScribbleVector		m_scribbles;

	bool				m_fScribbling;

	CLoader				m_loader;

};

#endif	//_H_GUI_H_
