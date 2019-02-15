#ifndef _ISAM_FILE_H_031105_
#define _ISAM_FILE_H_031105_

#include "FileEngine.h"

class CIsamFile : public CFileEngine
{
public:
	string m_sIndexFileName;
	FILE *fpDataFile;
	FILE *fpIdxFile;

public:
	CIsamFile();
	CIsamFile(string str);
	CIsamFile(string strData, string strIndex);
	virtual ~CIsamFile();

	int GetFileType() { return ISAM; }
	virtual bool Write(void *arg);
	bool Open(string strData, string strIndex);
	virtual void Close();

	bool Open(string str) { return false; }

};

#endif /* _ISAM_FILE_H_031105_ */
