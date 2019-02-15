#ifndef _Crawl_H_031104_
#define _Crawl_H_031104_

//#include <openssl/md5.h>
#include <zlib.h>

#include "Tse.h"
#include "Http.h"
#include "StrFun.h"
#include "Url.h"
#include "Page.h"
#include "TianwangFile.h"
#include "IsamFile.h"
#include "Link4SEFile.h"

using namespace std;

class CCrawl
{
public:
	string m_sInputFileName;	// seed URL file name
	string m_sOutputFileName;	// the file for saving parsed links

	CIsamFile m_isamFile;		// ISAM file handle
	ofstream m_ofsVisitedUrlFile;	// visited url file handle
	ofstream m_ofsLink4SEFile;	// link4SE url file handle
	ofstream m_ofsLink4HistoryFile;	// link4History url file handle
	ofstream m_ofsUnreachHostFile;	// unreach host file handle
	ofstream m_ofsVisitedUrlMD5File;// visited url MD5 file handle
	ofstream m_ofsVisitedPageMD5File;// visited url MD5 file handle
	ofstream m_ofsUnreachUrlFile;	// unreach URL file handle


public:
	CCrawl();
	CCrawl(string strInputFile, string strOutputFile);
	~CCrawl();

	// the main function for crawl
	void DoCrawl();

	// download the web pages
	void DownloadFile( CTianwangFile *pTianwangFile,
		CLink4SEFile *pLink4SEFile, CUrl iUrl, int& nGSock);

	// fetch the web pages. Each thread just execute this function.
	void fetch(void *arg);

	// add a parsed url into the collection
	void AddUrl(const char *url);

	void GetVisitedUrlMD5();
	void GetVisitedPageMD5();

	void GetIpBlock();
	//void GetUnreachHost();
	void GetUnreachHostMD5();
	void OpenFilesForOutput();

	// save in the process
	void SaveTianwangRawData(CTianwangFile *pTianwangFile,
			CUrl *pUrl, CPage *pPage);
	void SaveLink4SERawData(CLink4SEFile *pLink4SEFile,
			CUrl *pUrl, CPage *pPage);

	void SaveIsamRawData(CUrl *pUrl, CPage *Page);
	void SaveVisitedUrl(string url);
	void SaveUnreachHost(string host);
	void SaveLink4SE(CPage *Page);
	bool SaveLink4SE031121(void *arg);
	void SaveLink4History(CPage *Page);

	// save while the program running
	void SaveVisitedUrlMD5(string md5);
	void SaveVisitedPageMD5(string md5);

};

#endif /* _CRAWL_H_031104_ */

