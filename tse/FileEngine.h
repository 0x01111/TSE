#ifndef _FILE_ENGINE_H_031104_
#define _FILE_ENGINE_H_031104_

#include "DataEngine.h"

enum fileengine_type
{
	ISAM,
	TIANWANG,
	LINK4SE
};

class CUrl;
class CPage;

struct file_arg
{
	CUrl *pUrl;
	CPage *pPage;
};

class CFileEngine : public CDataEngine
{
public:
	ofstream m_ofsFile;

public:
	CFileEngine();
	CFileEngine(string str);
	virtual ~CFileEngine();

	int GetEngineType() { return FILE_ENGINE; }
	virtual int GetFileType() = 0;

	bool Open(string str);

	inline void Close() { m_ofsFile.close(); }

};

#endif /* _FILE_ENGINE_H_031104_ */
