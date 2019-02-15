
#include "FileEngine.h"

CFileEngine::CFileEngine()
{
}

CFileEngine::CFileEngine(string str) : CDataEngine (str)
{
	//LB_c: 打开文件输出流，注意ios::app表示追加模式
	m_ofsFile.open(m_str.c_str(), ios::out|ios::app|ios::binary);

	if( !m_ofsFile ){
		cerr << "cannot open " << m_str << "for output" << endl;
	}
}

CFileEngine::~CFileEngine()
{
	m_ofsFile.close();
}

bool CFileEngine::Open(string str)
{
	m_str = str;

	m_ofsFile.open(m_str.c_str(), ios::out|ios::app|ios::binary);

	if( !m_ofsFile ){
		cerr << "cannot open " << m_str << "for output" << endl;
		return false;
	} else {
		return true;
	}
}
