#ifndef _DATABASE_ENGINE_H_031104_
#define _DATABASE_ENGINE_H_031104_

#include "DataEngine.h"

class CDatabaseEngine : public CDataEngine
{
public:
	CDatabaseEngine(string str);
	virtual ~CDatabaseEngine();

	int GetEngineType() { return DATABASE_ENGINE; }

};

#endif /*_DATABASE_ENGINE_H_031104_*/
