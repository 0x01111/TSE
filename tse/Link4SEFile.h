#ifndef _LINK4SE_FILE_H_031208_
#define _LINK4SE_FILE_H_031208_

#include "Tse.h"
#include "Url.h"
#include "Page.h"
#include "FileEngine.h"

class CLink4SEFile : public CFileEngine
{
public:
	CLink4SEFile(string str);
	CLink4SEFile();
	virtual ~CLink4SEFile();

	inline int GetFileType() { return LINK4SE; }

	virtual bool Write(void *arg);
};

#endif /* _LINK4SE_FILE_H_031208_ */
