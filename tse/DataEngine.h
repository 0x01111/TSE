#ifndef _DATA_ENGINE_H_031104
#define _DATA_ENGINE_H_031104

#include "Tse.h"

enum dataengine_type
{
	FILE_ENGINE,
	DATABASE_ENGINE
};

class CDataEngine
{
public:
	//LB_c: 该类为抽象类，子类如果是DATABASE_ENGINE类型，则m_str表示数据库连接字符串，
	// 如果是FILE_ENGINE，则m_str表示文件名(带路径)
	string m_str;	// database engine ---connecting string
					// file engine ---file path & name

public:
	CDataEngine(string str);
	CDataEngine();
	virtual ~CDataEngine();

	virtual int GetEngineType() = 0;
	virtual bool Write(void *arg) = 0;
	virtual bool Open(string str) = 0;
	virtual void Close() = 0;
};

#endif /*_DATA_ENGINE_H_031104*/
