#include "Scribble.h"

#include <algorithm>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////

CScribble::CScribble()
{
	m_pObjectPoints.clear();
	m_pImagePoints.clear();

	m_nPoints	= 0;
	m_nID		= -1;
}

///////////////////////////////////////////////////////////////////////////////////////////

CScribble::CScribble(int nID)
{
	m_pObjectPoints.clear();
	m_pImagePoints.clear();

	m_nPoints	= 0;
	m_nID		= nID;
}

///////////////////////////////////////////////////////////////////////////////////////////

CScribble::~CScribble()
{
	m_pObjectPoints.clear();
	m_pImagePoints.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////

void CScribble::AddImagePoint(CPointInt & point)
{
	m_pImagePoints.push_back(point);
	//m_nPoints++;
}

///////////////////////////////////////////////////////////////////////////////////////////

void CScribble::AddImagePoint(int x, int y)
{
	CPointInt p(x,y);
	AddImagePoint(p);
}

///////////////////////////////////////////////////////////////////////////////////////////

void CScribble::AddObjectPoint(CPointFloat & point)
{
	m_pObjectPoints.push_back(point);
	m_nPoints++;
}

///////////////////////////////////////////////////////////////////////////////////////////

void CScribble::AddObjectPoint(float x, float y)
{
	CPointFloat p(x,y);
	AddObjectPoint(p);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CScribble::Load(ifstream &ifs)
{
	int i;
	CPointFloat pF;
	CPointInt	pI;
	char buffer[256];

	//clear all scribble points
	m_pObjectPoints.clear();
	m_pImagePoints.clear();

	if (ifs.eof())
		return false;

	ifs.getline(buffer, 255);
	sscanf(buffer, "<ID=%d> <PointsNum=%d>", &m_nID, &m_nPoints);
	ifs.getline(buffer, 255);
	sscanf(buffer, "<ObjectPoints>");

	for (i = 0; i < m_nPoints; i++)
	{
		float x = 0, y = 0;

		ifs.getline(buffer, 255, ' ');
		sscanf(buffer, "(%f,%f)", &x,&y);

		pF.x = x;
		pF.y = y;

		m_pObjectPoints.push_back(pF);
	}

	ifs.getline(buffer, 255);
	sscanf(buffer, "<ImagePoints>");

	for (i = 0; i < m_nPoints; i++)
	{
		int x = 0, y = 0;

		ifs.getline(buffer, 255, ' ');
		sscanf(buffer, "(%d,%d)", &x,&y);

		pI.x = x;
		pI.y = y;

		m_pImagePoints.push_back(pI);
	}

	//remove the last \n
	ifs.getline(buffer, 255);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CScribble::Save(char * pszFilename)
{
	int i;
	ofstream ofs;
	CPointFloat pF;
	CPointInt	pI;

	ofs.open(pszFilename, ios::app | ios::out);
	if (!ofs.is_open())
		return false;

	ofs << "<ID=" << m_nID << "> ";
	ofs << "<PointsNum=" << m_nPoints << ">\n";

	ofs << "<ObjectPoints>\n";

	for (i = 0; i < m_nPoints; i++)
	{
		float x, y;

		pF = m_pObjectPoints[i];

		x = pF.x;
		y = pF.y;

		ofs << "(" << x << "," << y << ") ";
	}
	ofs << "\n";

	ofs << "<ImagePoints>\n";
	for (i = 0; i < m_nPoints; i++)
	{
		int x, y;
		pI = m_pImagePoints[i];

		x = pI.x;
		y = pI.y;

		ofs << "(" << x << "," << y << ") ";
	}
	ofs << "\n";
	
	ofs.close();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

void CScribble::Draw() const
{
	for (int i = 0; i < m_nPoints; i++)
		glVertex3f( m_pObjectPoints[i].x, m_pObjectPoints[i].y, -0.2);
}

///////////////////////////////////////////////////////////////////////////////////////////

CPointInt & CScribble::operator[](int i)
{
	assert(0 <= i && i < m_pImagePoints.size());
	return m_pImagePoints[i];
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CScribble::Find(CPointInt p)	
{ 
	return find(m_pImagePoints.begin(), m_pImagePoints.end(), p)!= m_pImagePoints.end(); 
}

///////////////////////////////////////////////////////////////////////////////////////////
