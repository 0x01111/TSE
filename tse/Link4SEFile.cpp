#include "Link4SEFile.h"

extern map<string,string> mapCacheHostLookup;
extern vector<string> vsParsedLinks;

CLink4SEFile::CLink4SEFile()
{
}

CLink4SEFile::CLink4SEFile(string str) : CFileEngine(str)
{
}

CLink4SEFile::~CLink4SEFile()
{
	m_ofsFile.close();
}

bool CLink4SEFile::Write(void * arg)
{
	if (!arg || !m_ofsFile) return false;

	if (vsParsedLinks.size()==0) return false;

	file_arg *pFile = (file_arg *)arg;

	CUrl *iUrl = pFile->pUrl;
	CPage *iPage = pFile->pPage;

	char strDownloadTime[128];
	time_t tDate;

	memset(strDownloadTime, 0, 128);
	time(&tDate);
	strftime(strDownloadTime, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));

	string links;
	vector<string>::iterator it;
	for (it=vsParsedLinks.begin(); it!=vsParsedLinks.end(); ++it)
	{
		links = links + *it + "\n";
	}

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

	m_ofsFile << "\noutdegree: " << vsParsedLinks.size();
	m_ofsFile << "\nlength: " << links.size() + iPage->m_nLenHeader + 1 
		<< "\n\n" << iPage->m_sHeader << "\n";

	m_ofsFile << links;
	m_ofsFile << endl;

	return true;
}
