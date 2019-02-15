#ifndef _TIANWANG_FILE_H_031104_
#define _TIANWANG_FILE_H_031104_

#include "Tse.h"
#include "Url.h"
#include "Page.h"
#include "FileEngine.h"

class CTianwangFile : public CFileEngine
{
public:
	CTianwangFile(string str);
	CTianwangFile();
	virtual ~CTianwangFile();

	inline int GetFileType() { return TIANWANG; }

	virtual bool Write(void *arg);
};

#endif /* _TIANWANG_FILE_H_031104_ */
