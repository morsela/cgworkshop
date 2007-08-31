#include "ControlPanel.h"

///////////////////////////////////////////////////////////////////////////////////
// CControlPanel
///////////////////////////////////////////////////////////////////////////////////

CControlPanel::CControlPanel()
{
	m_width = 60;
	m_colorBoxes.clear();
	m_lineBoxes.clear();
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::Reset()
{
	for (unsigned int i = 0; i < m_colorBoxes.size(); i++)
		m_colorBoxes[i].Reset();
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::AddColorBox(int nID, CvScalar color)
{
	if (m_nColorRow > COLUMN_NUMBER)
	{
		m_nColorCol++;
		m_nColorRow = 1;
	}

	CColorBox colorBox(nID, m_x + MARGIN_SIZE + BOX_MARGIN_SIZE * (m_nColorRow - 1) + COLOR_BOX_SIZE * (m_nColorRow - 1), 
		m_y - MARGIN_SIZE - BOX_MARGIN_SIZE * (m_nColorCol - 1) - COLOR_BOX_SIZE * (m_nColorCol - 1), 
		COLOR_BOX_SIZE,COLOR_BOX_SIZE, color);
	m_colorBoxes.push_back(colorBox);
	m_nColorRow++;
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::AddLineBox(int nID, int nLineWidth)
{
	CLineBox lineBox(nID, m_x + (m_x + m_width - m_x)/2 - LINE_BOX_WIDTH / 2,//m_x + MARGIN_SIZE, 
		m_y - MARGIN_SIZE - BOX_MARGIN_SIZE * (m_nColorCol - 1) - COLOR_BOX_SIZE * (m_nColorCol - 1) - LINE_BOX_MARGIN
		- BOX_MARGIN_SIZE * (m_nLineRow - 1) -  LINE_BOX_HEIGHT * (m_nLineRow - 1),
		//m_y - m_height/2 - BOX_MARGIN_SIZE * (m_nLineRow - 1) -  LINE_BOX_HEIGHT * (m_nLineRow - 1),
		LINE_BOX_WIDTH, LINE_BOX_HEIGHT, nLineWidth);

	m_lineBoxes.push_back(lineBox);

	m_nLineRow++;
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::Setup(float x, float y, int height) 
{ 
	m_nColorRow = 1; m_nColorCol = 1; m_nLineRow = 1;
	m_x = x; m_y = y;
	m_height = height;

	for (int i = 0; i < SCRIBBLE_NUMBER; i++)
		AddColorBox(i + COLOR_OFFSET,s_colors[i]);

	m_nColorCol++;

	for (int i = 0; i < LINE_WIDTH_NUMBER; i++)
		AddLineBox(i + LINE_OFFSET, 2*i + 1);
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::Draw()
{
	for (unsigned int i = 0; i < m_colorBoxes.size(); i++)
		m_colorBoxes[i].Draw();

	for (unsigned int i = 0; i < m_lineBoxes.size(); i++)
		m_lineBoxes[i].Draw();

	//draw background
	glBegin(GL_QUADS);										// Draw A Quad
		glColor3f(0.8,0.8,0.8);
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);			// Top Left
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Top Right
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Bottom Right
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);				// Bottom Left
	glEnd();	
}

///////////////////////////////////////////////////////////////////////////////////

void CControlPanel::Choose(float x, float y)
{
	int nChosenColorBox = UNDEFINED;
	int nChosenLineBox	= UNDEFINED;

	//////////
	// Choose Color Box
	for (unsigned int i = 0; i < m_colorBoxes.size(); i++)
	{
		if (m_colorBoxes[i].IsChosen())
		{
			nChosenColorBox = i;
			break;
		}
	}

	for (unsigned int i = 0; i < m_colorBoxes.size(); i++)
		if (m_colorBoxes[i].Choose(x,y))
			nChosenColorBox = i;

	if (nChosenColorBox != UNDEFINED)
		m_colorBoxes[nChosenColorBox].Choose();
	
	//////////
	// Choose Line Box
	for (unsigned int i = 0; i < m_lineBoxes.size(); i++)
	{
		if (m_lineBoxes[i].IsChosen())
		{
			nChosenLineBox = i;
			break;
		}
	}

	for (unsigned int i = 0; i < m_lineBoxes.size(); i++)
		if (m_lineBoxes[i].Choose(x,y))
			nChosenLineBox = i;

	if (nChosenLineBox != UNDEFINED)
		m_lineBoxes[nChosenLineBox].Choose();
	else
		m_lineBoxes[0].Choose();
}

///////////////////////////////////////////////////////////////////////////////////

int CControlPanel::GetChosenColor(CvScalar & color)
{
	for (unsigned int i = 0; i < m_colorBoxes.size(); i++)
	{
		if (m_colorBoxes[i].IsChosen() && m_colorBoxes[i].GetType() == CBox::TYPE_COLOR_BOX)
		{
			color = m_colorBoxes[i].GetColor();
			return m_colorBoxes[i].GetID();
		}
	}
	return UNDEFINED;
}

///////////////////////////////////////////////////////////////////////////////////

int CControlPanel::GetChosenLineWidth(int &nLineWidth)
{
	for (unsigned int i = 0; i < m_lineBoxes.size(); i++)
	{
		if (m_lineBoxes[i].IsChosen() && m_lineBoxes[i].GetType() == CBox::TYPE_LINE_BOX)
		{
			nLineWidth = m_lineBoxes[i].GetLineWidth();
			return m_lineBoxes[i].GetID();
		}
	}
	//should never happen
	return UNDEFINED;
}

///////////////////////////////////////////////////////////////////////////////////
