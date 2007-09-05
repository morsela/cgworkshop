#ifndef _H_BOX_H_
#define _H_BOX_H_

#include <cv.h>
#include <GL/glut.h>

#define COLOR_OFFSET	0
#define LINE_OFFSET		100
#define BUTTON_OFFSET	200

#define CHOSSEN_COLOR_BOX_MARGIN	2.0

#define MARGIN_SIZE		8
#define BOX_MARGIN_SIZE	5

#define	UNDEFINED	-1

class CBox
{
public:

	enum EBoxType
	{
		TYPE_COLOR_BOX,
		TYPE_LINE_BOX,
		TYPE_BUTTON_BOX
	};

public:
	CBox(int nID, float x, float y, float width, float height, EBoxType type):
	  m_x(x),m_y(y),m_width(width),m_height(height),m_fChosen(false),m_nID(nID),m_type(type)
	  {}

	  ~CBox() {}

	  virtual void Draw() = 0;

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

	  void Draw();

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

	  void Draw();

	  int GetLineWidth()	const { return m_nLineWidth; }

protected:

	int	m_nLineWidth;
};

class CButtonBox : public CBox
{
public:
	enum EButtonCommand
	{
		command_Colorize_A = 0,
		command_Colorize_B,
		command_Colorize_C,
		command_Clear,
		command_Save,
		command_Load,
		command_Quit,
		command_Max
	};

protected:
	static const int INNER_LINE_MARGIN_WIDTH	= 5;
	static const int TEXT_MARGIN				= 2;

public:
	CButtonBox(int nID, float x, float y, float width, float height, EButtonCommand command, char * cmdString):
	  CBox(nID,x,y,width, height, TYPE_BUTTON_BOX), m_command(command)
	  {
		  strcpy(m_pszString, cmdString);
	  }

	  ~CButtonBox() {}

	  void Draw();

	  int GetCommand()	const { return m_command; }

protected:

	char 			m_pszString[256];
	EButtonCommand	m_command;
};
#endif	//_H_BOX_H_