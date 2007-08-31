#include "ButtonPanel.h"

///////////////////////////////////////////////////////////////////////////////////

CButtonPanel::CButtonPanel()
{
	m_height = 30;
	m_nButtonRow = 0;
}

///////////////////////////////////////////////////////////////////////////////////

void CButtonPanel::AddButtonBox(int nID, CButtonBox::EButtonCommand cmd)
{
	CButtonBox buttonBox(nID, m_x + MARGIN_SIZE + BOX_MARGIN_SIZE * (m_nButtonRow - 1) + BUTTON_BOX_WIDTH * (m_nButtonRow - 1), 
		m_y - Y_MARGIN_SIZE, BUTTON_BOX_WIDTH,BUTTON_BOX_HEIGHT, cmd, buttonDefs[cmd].cmdString);
	m_buttonBoxes.push_back(buttonBox);
	m_nButtonRow++;
}

///////////////////////////////////////////////////////////////////////////////////

void CButtonPanel::Setup(float x, float y, int width) 
{ 
	m_nButtonRow = 1;
	m_x = x; m_y = y;
	m_width = width;

	for (int i = 0; i < CButtonBox::command_Max; i++)
		AddButtonBox(i + COLOR_OFFSET,(CButtonBox::EButtonCommand)i);
}

///////////////////////////////////////////////////////////////////////////////////

void CButtonPanel::Draw()
{
	for (unsigned int i = 0; i < m_buttonBoxes.size(); i++)
		m_buttonBoxes[i].Draw();

	//draw background
	glBegin(GL_QUADS);										// Draw A Quad
		glColor3f(0.8,0.8,0.8);
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);			// Top Left
		glVertex3f(m_x + m_width*2 + CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Top Right
		glVertex3f(m_x + m_width*2 + CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Bottom Right
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);				// Bottom Left
	glEnd();	
}

///////////////////////////////////////////////////////////////////////////////////

void CButtonPanel::Choose(float x, float y)
{
	for (unsigned int i = 0; i < m_buttonBoxes.size(); i++)
		m_buttonBoxes[i].Choose(x,y);
}

///////////////////////////////////////////////////////////////////////////////////

int CButtonPanel::GetChosenButton(CButtonBox::EButtonCommand & cmd)
{
	for (unsigned int i = 0; i < m_buttonBoxes.size(); i++)
	{
		if (m_buttonBoxes[i].IsChosen() && m_buttonBoxes[i].GetType() == CBox::TYPE_BUTTON_BOX)
		{
			cmd = (CButtonBox::EButtonCommand) m_buttonBoxes[i].GetCommand();
			return m_buttonBoxes[i].GetID();
		}
	}
	return UNDEFINED;
}

///////////////////////////////////////////////////////////////////////////////////
