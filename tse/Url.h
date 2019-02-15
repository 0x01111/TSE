#ifndef _URL_H_030728_
#define _URL_H_030728_

#include <string>

const unsigned int URL_LEN	= 256;
const unsigned int HOST_LEN	= 256;

using namespace std;


enum url_scheme {
	SCHEME_HTTP,
	SCHEME_FTP,
	SCHEME_INVALID
};

const int DEFAULT_HTTP_PORT = 80;
const int DEFAULT_FTP_PORT  = 21;

class CUrl
{
public:
	string m_sUrl;			// Original URL
	enum url_scheme m_eScheme;	// URL scheme

	string	m_sHost;		// Extracted hostname 
	int	m_nPort;		// Port number
	string	m_sPath;		// Request

	/*
	// URL components (URL-quoted). 
	string	m_sPath,
		m_sParams,
		m_sQuery,
		m_sFragment;

	// Extracted path info (unquoted). 
	string	m_sDir,
		m_sFile;

	// Username and password (unquoted). 
	string	m_sUser,
		m_sPasswd;
	*/

public:
	CUrl();
	~CUrl();

	//bool ParseUrl(string strUrl);

	// break  an URL into scheme, host, port and request.
	// result as member variants
	bool ParseUrlEx(string strUrl);

	// break an URL into scheme, host, port and request.
	// result url as argvs
	void ParseUrlEx(const char *url, char *protocol, int lprotocol,
			char *host, int lhost,
			char *request, int lrequest, int *port);

	// get the ip address by host name
	char *GetIpByHost(const char *host);

	bool IsValidHost(const char *ip);
	bool IsForeignHost(string host);
	bool IsImageUrl(string url);
	bool IsValidIp(const char *ip);
	bool IsVisitedUrl(const char *url);
	bool IsUnReachedUrl(const char *url);
	bool IsValidHostChar(char ch);

private:
	void ParseScheme (const char *url);

};

extern pthread_mutex_t mutexMemory;

#endif /* _URL_H_030728_ */

