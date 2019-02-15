#include "TianwangFile.h"

extern map<string,string> mapCacheHostLookup;

CTianwangFile::CTianwangFile()
{
}

CTianwangFile::CTianwangFile(string str) : CFileEngine(str)
{
}

CTianwangFile::~CTianwangFile()
{
	m_ofsFile.close();
}

//LB_c: 将网页内容及相关数据写入天网格式文件
bool CTianwangFile::Write(void * arg)
{
	if( !arg || !m_ofsFile ){
		return false;
	}

	file_arg *pFile = (file_arg *)arg;

	CUrl *iUrl = pFile->pUrl;
	CPage *iPage = pFile->pPage;

	char strDownloadTime[128];
	time_t tDate;
	memset(strDownloadTime, 0, 128);
	time(&tDate);
	strftime(strDownloadTime, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));

	m_ofsFile << "version: 1.0\n";
	if( iPage->m_sLocation.length() == 0 ){
		m_ofsFile << "url: " << iUrl->m_sUrl;
	} else {
		m_ofsFile << "url: " << iPage->m_sLocation;
		m_ofsFile << "\norigin: " << iUrl->m_sUrl;
	}

	m_ofsFile << "\ndate: " << strDownloadTime;
	if( mapCacheHostLookup.find(iUrl->m_sHost) == mapCacheHostLookup.end() ){
		m_ofsFile << "\nip: " << iUrl->m_sHost;
	} else {
		m_ofsFile << "\nip: " << ( *(mapCacheHostLookup.find(iUrl->m_sHost)) ).second;
	}

	m_ofsFile << "\nlength: " << iPage->m_nLenContent + iPage->m_nLenHeader + 1 
		<< "\n\n" << iPage->m_sHeader << "\n";

	m_ofsFile.write( iPage->m_sContent.c_str(), iPage->m_nLenContent );
	m_ofsFile << endl;

	return true;
}
