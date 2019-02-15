#ifndef _Page_H_030728_
#define _Page_H_030728_

#include <string>
#include <map>
#include <vector>
#include <list>
#include "Url.h"

#include <list.h>
#include <hlink.h>
#include <uri.h>

//large enough to hold sina's 437 links 

const int ANCHOR_TEXT_LEN       = 256;
const int MAX_URL_REFERENCES    = 1000;
const int URL_REFERENCE_LEN     = (URL_LEN+ANCHOR_TEXT_LEN)*MAX_URL_REFERENCES*1/2 ;
const int MAX_TAG_NUMBERS	= 10000;

using namespace std;

// plain text or other
enum page_type {
	PLAIN_TEXT,
	OTHER	
};

struct RefLink4SE	// <href src...>, <area src...>
{
	char *link;
	char *anchor_text;
	string strCharset;
};

struct RefLink4History	// <img src...>,<script src...>
{
	char *link;
};

class CPage
{
public:
	// url & location
	string m_sUrl;		

	// header
	string m_sHeader;
	int m_nLenHeader;

	int m_nStatusCode;
	int m_nContentLength;
	string m_sLocation;
	bool m_bConnectionState;	// if "Connection: close" false, otherwise true
	string m_sContentEncoding;
	string m_sContentType;
	string m_sCharset;
	string m_sTransferEncoding;

	// content
	string m_sContent;
	int m_nLenContent;
	string m_sContentNoTags;


	// link, in a lash-up state
	string m_sContentLinkInfo;

	// links for SE, in a lash-up state
	string m_sLinkInfo4SE;
	int m_nLenLinkInfo4SE;

	// links for history archiving, in a lash-up state
	string m_sLinkInfo4History;
	int m_nLenLinkInfo4History;


	// links for SE, in a good state
	RefLink4SE m_RefLink4SE[MAX_URL_REFERENCES];
	int m_nRefLink4SENum;

	// links for history archiving, in a good state
	RefLink4History m_RefLink4History[MAX_URL_REFERENCES/2];
	int m_nRefLink4HistoryNum;

	//map<string,string,less<string> > m_mapLink4SE;
	map<string,string> m_mapLink4SE;
	vector<string > m_vecLink4History;

	// page type
	enum page_type m_eType;

	// parsed url lists
	//list<string>	m_listLink4SE;

public:
	CPage();
	CPage(string strUrl, string strLocation, char* header, char* body, int nLenBody);
	~CPage();

	// parse header information from the header content
	void ParseHeaderInfo(string header);

	// parse hyperlinks from the page content
	bool ParseHyperLinks();

	bool NormalizeUrl(string& strUrl);

	bool IsFilterLink(string plink);

private:
	// parse header information from the header content
	void GetStatusCode(string header);
	void GetContentLength(string header);
	void GetConnectionState(string header);
	void GetLocation(string header);
	void GetCharset(string header);
	void GetContentEncoding(string header);
	void GetContentType(string header);
	void GetTransferEncoding(string header);

	// parse hyperlinks from the web page
	bool GetContentLinkInfo();

	bool GetLinkInfo4SE();
	bool GetLinkInfo4History();
	bool FindRefLink4SE();
	bool FindRefLink4History();

};

#endif /* _Page_H_030728_ */

