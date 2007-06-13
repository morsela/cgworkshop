#include <fstream>

#include "Loader.h"

///////////////////////////////////////////////////////////////////////////////////

void CLoader::Setup(const char * pScribbleFile)
{
	m_fInitialized = true;
	strncpy(m_pScribbleFile, pScribbleFile, MAX_FILE_LENGTH*sizeof(char));
}

///////////////////////////////////////////////////////////////////////////////////

void CLoader::Load(ScribbleVector & scribbles)
{
	if (!m_fInitialized)
		return;

	std::ifstream ifs;
	CScribble scribble;

	ifs.open(m_pScribbleFile);
	if (!ifs.is_open())
		return;

	scribbles.clear();

	//load all the scribbles from the file
	while (true)
	{
		bool fLoad = scribble.Load(ifs);
		if (ifs.eof())
			break;

		scribbles.push_back(scribble);
	}

	ifs.close();
}

///////////////////////////////////////////////////////////////////////////////////

void CLoader::Save(ScribbleVector & scribbles)
{
	if (!m_fInitialized)
		return;

	std::ofstream ofs;

	//Truncate the scribble file
	ofs.open(m_pScribbleFile, std::ios::trunc);
	ofs.close();

	//save all scribbles
	for (int i = 0; i < scribbles.size(); i++)
		scribbles[i].Save(m_pScribbleFile);
}

///////////////////////////////////////////////////////////////////////////////////
