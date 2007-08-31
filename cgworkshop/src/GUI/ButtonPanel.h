#pragma once

#include <vector>
#include "Box.h"

#define Y_MARGIN_SIZE	5

#define BUTTON_BOX_WIDTH	140
#define BUTTON_BOX_HEIGHT	20

struct SButton
{
	CButtonBox::EButtonCommand	cmd;
	char *			cmdString;
};

const SButton buttonDefs[CButtonBox::command_Max] = 
					{	{ CButtonBox::command_Colorize, "Colorize" },
						{ CButtonBox::command_Clear, "Clear" },
						{ CButtonBox::command_Save, "Save" },
						{ CButtonBox::command_Load, "Load" },
						{ CButtonBox::command_Quit, "Quit" }	};

class CButtonPanel
{
public:
	CButtonPanel();
	~CButtonPanel() {}

	void AddButtonBox(int nID, CButtonBox::EButtonCommand command);

	void Setup(float x, float y, int width);

	void	Choose(float x, float y);

	void	Draw();

	int		GetChosenButton(CButtonBox::EButtonCommand & cmd);

	int GetHeight() const { return m_height; }

protected:

	float m_x;
	float m_y;

	float	m_width;
	float	m_height;		

	int m_nButtonRow;

	std::vector<CButtonBox>	m_buttonBoxes;

};
