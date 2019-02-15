#ifndef _Document_H_040410_
#define _Document_H_040410_

#include <string>

typedef struct{
        int docid;
        int offset;
}DocIdx;

using namespace std;

class CDocument
{
public:

	int m_nDocId;
	int m_nPos;
	int m_nLength;
	string m_sChecksum;

	string m_sUrl;
	string m_sRecord;	// a record including a HEAD, a header and body
	string m_sHead;
	string m_sHeader;
	string m_sBody;

	string m_sBodyNoTags;

public:
	CDocument();
	~CDocument();

	bool ParseRecord(string &content) const;
	bool CleanBody(string &body) const;

	void RemoveTags(char *s);
};

#endif /* _Document_H_040410_ */
