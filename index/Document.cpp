/*Document handling
 */

#include "Document.h"

CDocument::CDocument()
{
	m_nDocId = -1;
	m_nPos = -1;
	m_nLength = 0;
	m_sChecksum = "";

	m_sUrl = "";
}

CDocument::~CDocument()
{
}

bool CDocument::ParseRecord(string &content) const
{
	return true;
}

bool CDocument::CleanBody(string &body) const
{
	return true;
}

//remove tag and content of scripts, css, java, embeddedobjects, comments, etc
void CDocument::RemoveTags(char *s)
{
	int intag;
	char *p, *q;

	if (!s || !*s)	return;

	for (p=q=s, intag=0; *q; q++) {
		switch (*q){
		case '<':
			intag = 1;
			*p++ = ' ';
			break;
		case '>':
			intag = 0;
			break;
		default:
			if (!intag) {
				*p++ = *q;
			}
			break;
		}
	}

	*p = '\0';

/* second method
	char *d = s;
	while (*s) {
		if (*s == '<') {
			while (*s && *s!='>') s++;
			if( *s == '\0') break;
			s++; 
			continue; 
		} 

		*d++ = *s++; 
	}
	*d = 0;
*/
}
