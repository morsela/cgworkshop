#ifndef _H_LOADER_H_
#define _H_LOADER_H_

#include "TypeDefs.h"

#define MAX_FILE_LENGTH	255

class CLoader
{
public:
	
	CLoader():m_fInitialized(false) {}

	virtual ~CLoader() {}

public:

	void Setup(const char * pScribbleFile);

	void Load(ScribbleVector & scribbles);
	
	void Save(ScribbleVector & scribbles);

protected:

	char	m_pScribbleFile[MAX_FILE_LENGTH];

	bool	m_fInitialized;

};

#endif	//_H_LOADER_H_
