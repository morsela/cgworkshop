#pragma once

#include <cv.h>
#include <vector>
#include <GL/glut.h>

//REMOVEME:
#include "TypeDefs.h"

#define LINE_WIDTH_NUMBER	5

#define	UNDEFINED	-1

#define CHOSSEN_COLOR_BOX_MARGIN	2.0

#define COLOR_OFFSET	0
#define LINE_OFFSET		100

enum EBoxType
{
	TYPE_COLOR_BOX,
	TYPE_LINE_BOX,
};

class CBox
{
public:
	CBox(int nID, float x, float y, float width, float height, EBoxType type):
		m_x(x),m_y(y),m_width(width),m_height(height),m_fChosen(false),m_nID(nID),m_type(type)
		{}

	~CBox() {}

	virtual void Draw() const = 0;

	void Reset()		{	m_fChosen = false; }

	void Choose()		{ m_fChosen = true; }
	bool Choose(float x, float y);
	bool		IsChosen()		{ return m_fChosen; }

	int			GetID()			{ return m_nID; }
	EBoxType	GetType()	{ return m_type; }

protected:

	EBoxType	m_type;
	int			m_nID;
	bool		m_fChosen;

	float		m_x;
	float		m_y;
	float		m_width;
	float		m_height;
};

class CColorBox : public CBox
{
public:
	CColorBox(int nID, float x, float y, float width, float height, CvScalar& color):
		CBox(nID,x,y,width, height,TYPE_COLOR_BOX), m_color(color)
		{}

	~CColorBox() {}

	void Draw() const;

	CvScalar & GetColor()	{ return m_color; }

protected:
	CvScalar	m_color;
};


class CLineBox : public CBox
{
protected:
	static const int INNER_LINE_MARGIN_WIDTH = 5;

public:
	CLineBox(int nID, float x, float y, float width, float height, int nLineWidth):
	  CBox(nID,x,y,width, height, TYPE_LINE_BOX), m_nLineWidth(nLineWidth)
	  {}

	  ~CLineBox() {}

	  void Draw() const;

	  int GetLineWidth()	const { return m_nLineWidth; }

protected:

	int	m_nLineWidth;
};

#define COLUMN_NUMBER	2
#define MARGIN_SIZE		8
#define BOX_MARGIN_SIZE	5
#define COLOR_BOX_SIZE	20

#define LINE_BOX_WIDTH	40
#define LINE_BOX_HEIGHT	20

static const CvScalar s_colors[] = {{{0,0,1.0f}},{{0,1.0f,0}},{{0,1.0f,1.0f}},{{1.0f,1.0f,0}},{{0,0,1.0f}},{{0.5f,0.5f,1.0f}}
,{{0.5f,0.5f,0.5f}},{{0.5f,1.0f,0.5f}},{{1.0f,0.5f,1.0f}},{{1.0f,0.2f,0.7f}}};

class CControlPanel
{
public:
	CControlPanel();
	virtual ~CControlPanel() {}

	void Reset();

	void AddColorBox(int nID, CvScalar color);

	void AddLineBox(int nID, int nLineWidth);

	void Setup(float x, float y, int height);

	void	Choose(float x, float y);

	void	Draw() const;

	int		GetWidth() { return m_width; }

	int		GetChosenColor(CvScalar & color);

	int		GetChosenLineWidth(int &nLineWidth);

protected:

	float m_x;
	float m_y;

	float	m_width;
	float	m_height;		

	int m_nColorRow;
	int m_nColorCol;

	int m_nLineRow;

	std::vector<CColorBox>	m_colorBoxes;

	std::vector<CLineBox>	m_lineBoxes;
};
