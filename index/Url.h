#ifndef _Url_H_040410_
#define _Url_H_040410_

#include <string>

using namespace std;

class CUrl
{
public:
	string m_sUrl;
	string m_sChecksum;
	int m_nDocId;

public:
	CUrl();
	~CUrl();
};

#endif /* _Url_H_040410_ */
