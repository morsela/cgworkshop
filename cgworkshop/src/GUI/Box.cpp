#include "Box.h"

///////////////////////////////////////////////////////////////////////////////////
// CBox
///////////////////////////////////////////////////////////////////////////////////

bool CBox::Choose(float x, float y)
{
	m_fChosen = ((x > m_x) && (x < m_x + m_width) && (y < m_y) && (y > m_y - m_height));
	return m_fChosen;
}

///////////////////////////////////////////////////////////////////////////////////
// CColorBox
///////////////////////////////////////////////////////////////////////////////////

void CColorBox::Draw()
{
	glBegin(GL_QUADS);										// Draw A Quad
	glColor3f(m_color.val[0],m_color.val[1],m_color.val[2]);
	glVertex3f(m_x, m_y, 0.0f);							// Top Left
	glVertex3f(m_x + m_width, m_y, 0.0f);				// Top Right
	glVertex3f(m_x + m_width, m_y - m_height, 0.0f);	// Bottom Right
	glVertex3f(m_x, m_y - m_height, 0.0f);				// Bottom Left
	glEnd();	
	//mark chosen box
	if (m_fChosen)
	{
		glBegin(GL_QUADS);										// Draw A Quad
		glColor3f(0.0,0.0,0.0);
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);			// Top Left
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Top Right
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Bottom Right
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);				// Bottom Left
		glEnd();	
	}
}

///////////////////////////////////////////////////////////////////////////////////
// CLineBox
///////////////////////////////////////////////////////////////////////////////////

void CLineBox::Draw()
{
	glBegin(GL_QUADS);
	glColor3f(0.0,0.0,0.0);
	glVertex3f(m_x + INNER_LINE_MARGIN_WIDTH, m_y - (m_height / 2) + (m_nLineWidth / 2), 0.0f);							// Top Left
	glVertex3f(m_x + m_width - INNER_LINE_MARGIN_WIDTH, m_y - (m_height / 2) + (m_nLineWidth / 2), 0.0f);				// Top Right
	glVertex3f(m_x + m_width - INNER_LINE_MARGIN_WIDTH, m_y - (m_height / 2) - (m_nLineWidth / 2), 0.0f);	// Bottom Right
	glVertex3f(m_x + INNER_LINE_MARGIN_WIDTH, m_y - (m_height / 2) - (m_nLineWidth / 2), 0.0f);				// Bottom Left
	glEnd();	

	glBegin(GL_QUADS);										// Draw A Quad
	glColor3f(0.5,0.5,0.5);
	glVertex3f(m_x, m_y, 0.0f);							// Top Left
	glVertex3f(m_x + m_width, m_y, 0.0f);				// Top Right
	glVertex3f(m_x + m_width, m_y - m_height, 0.0f);	// Bottom Right
	glVertex3f(m_x, m_y - m_height, 0.0f);				// Bottom Left
	glEnd();	
	//mark chosen box
	if (m_fChosen)
	{
		glBegin(GL_QUADS);										// Draw A Quad
		glColor3f(0.0,0.0,0.0);
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);			// Top Left
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y + CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Top Right
		glVertex3f(m_x + m_width + CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);	// Bottom Right
		glVertex3f(m_x - CHOSSEN_COLOR_BOX_MARGIN, m_y - m_height - CHOSSEN_COLOR_BOX_MARGIN, 0.0f);				// Bottom Left
		glEnd();	
	}
}

///////////////////////////////////////////////////////////////////////////////////
// CButtonBox
///////////////////////////////////////////////////////////////////////////////////

int MeasureText(char * buffer, void * font)
{
	int nWidth = 0;

	// For each character in the string... draw it!
	for (int i = 0; i < ((int) strlen(buffer)); i++)
		nWidth += glutBitmapWidth(font, buffer[i]);
	return nWidth;
}

///////////////////////////////////////////////////////////////////////////////////

void DrawText(int x, int y, char * buffer, void * font)
{
	glColor3f(0.0,0.0,0.0);

	glRasterPos2f(x, y);

	// For each character in the string... draw it!
	for (int i = 0; i < ((int) strlen(buffer)); i++)
		glutBitmapCharacter(font, buffer[i]);
}

///////////////////////////////////////////////////////////////////////////////////

void CButtonBox::Draw()
{
	int nWidth = MeasureText(m_pszString, GLUT_BITMAP_HELVETICA_12);
	DrawText( m_x + (m_width / 2) - (nWidth / 2), m_y - 14, m_pszString, GLUT_BITMAP_HELVETICA_12);

	glBegin(GL_QUADS);										// Draw A Quad
	//mark chosen box
	if (m_fChosen)
		glColor3f(0.3,0.3,0.3);
	else
		glColor3f(0.5,0.5,0.5);
	
	glVertex3f(m_x, m_y, 0.0f);							// Top Left
	glVertex3f(m_x + m_width, m_y, 0.0f);				// Top Right
	glVertex3f(m_x + m_width, m_y - m_height, 0.0f);	// Bottom Right
	glVertex3f(m_x, m_y - m_height, 0.0f);				// Bottom Left
	glEnd();
}

///////////////////////////////////////////////////////////////////////////////////
