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


	//load all the scribbles from the file
	for (int i = 0; i < SCRIBBLE_NUMBER; i++)
	{
		scribbles[i].Reset();
		bool fLoad = scribbles[i].Load(ifs);
		if (ifs.eof() || !fLoad)
			continue;
	}

	ifs.close();

	printf("Load operation succeeded\n");
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
	for (unsigned int i = 0; i < scribbles.size(); i++)
		scribbles[i].Save(m_pScribbleFile);

	printf("Save operation succeeded\n");
}

///////////////////////////////////////////////////////////////////////////////////
