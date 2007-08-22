#ifndef _H_GUI_H_
#define _H_GUI_H_

#include <cv.h>

#include <GL/glut.h>

#include "TypeDefs.h"
#include "Loader.h"
#include "ControlPanel.h"

class CGUI
{
public:
	
	CGUI();
	virtual ~CGUI() {}

	bool Setup(char * pszImagePath, char * pScribbleFile = NULL);

	static CGUI * GetInstance() { static CGUI inst; return &inst; }

	void LoadTextures();

public:

	int GetWindowWidth() const { return m_nWindowWidth; }
	
	int GetWindowHeight() const { return m_nWindowHeight; }

	void SetWindowSize(int x, int y) { m_nWindowWidth = x; m_nWindowHeight = y; }

	int GetImageWidth() const { return m_nImageWidth; }

	int GetImageHeight() const { return m_nImageHeight; }

	void SetImageSize(int x, int y) { m_nImageWidth = x; m_nImageHeight = y; }

	const IplImage * const GetImage() { return m_pImg; }

public:

	void MouseMove(int x, int y);
	
	void MouseAction(int button, int state, int x, int y);

	void KeysAction(unsigned char key, int x, int y );

	void Reshape(int x , int y);

	void Render();

protected:
	
	void AddScribblePoints(int x, int y);

	friend class CControlPanel;

protected:
	
	CControlPanel		m_ctrlPanel;

	IplImage *			m_pImg;

	int					m_nWindowWidth;

	int					m_nWindowHeight;

	int					m_nImageWidth;

	int					m_nImageHeight;

	// the bounding box for the opengl orthogonal projection
	float 				m_orthoBBox[4];

	float 				m_orthoBBox2[4];

	unsigned int		m_textures[1];

	// The vectors in object space coordinates (for rendering)
	ScribbleVector		m_scribbles;

	bool				m_fScribbling;

	CLoader				m_loader;

	int					m_nCurScribble;

	int					m_nScribblesNum;

};

#endif	//_H_GUI_H_
