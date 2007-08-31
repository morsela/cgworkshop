#pragma once

#include <vector>

//REMOVEME:
#include "TypeDefs.h"

#include "Box.h"

#define LINE_WIDTH_NUMBER	5

#define COLUMN_NUMBER	2

#define COLOR_BOX_SIZE	20

#define LINE_BOX_WIDTH	40
#define LINE_BOX_HEIGHT	15

#define LINE_BOX_MARGIN	5

static const CvScalar s_colors[] = { {{0.415,0.25,0.0f}},{{1.0,0.5f,0}},
									{{0,0,0.4f}}, {{0,0,1.0f}},
									{{0,1.0f,1.0f}},{{1.0f,1.0f,0}},
									{{0.5f,0.5f,1.0f}},{{1.0f,0.5f,1.0f}},
									{{0.5f,0.5f,0.5f}},{{0.5f,0.8f,0.5f}} };
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

	void	Draw();

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
