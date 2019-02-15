#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "Md5.h"
#include "Url.h"
#include "Document.h"
#include "ChSeg/Dict.h"
#include "ChSeg/HzSeg.h"
#include "StrFun.h"

CDict iDict;

using namespace std;

const unsigned int HEADER_BUF_SIZE = 1024;
//const unsigned int MAX_DOC_ID = 12932;		// you should change according "Doc.idx"
const unsigned int MAX_DOC_ID = 21312;

int main(int argc, char* argv[])
{
	string strLine, strFileName=argv[1];
	CUrl iUrl;
	vector<CUrl> vecCUrl;
	CDocument iDocument;
	vector<CDocument> vecCDocument;
	unsigned int docId = 0;

	//ifstream ifs("Tianwang.raw.2559638448");
	ifstream ifs(strFileName.c_str());
	if (!ifs) {
		cerr << "Cannot open tianwang.img.info for input\n";
                return -1;
        }

	ifstream ifsUrl("Url.idx.sort_uniq");
        if (!ifsUrl) {
                cerr << "Cannot open Url.idx.sort_uniq for input\n";
                return -1;
        }
	ifstream ifsDoc("Doc.idx");
        if (!ifsDoc) {
                cerr << "Cannot open Doc.idx for input\n";
                return -1;
        }

	while (getline(ifsUrl,strLine)){
		char chksum[33];
		int  docid;

		memset(chksum, 0, 33);
		sscanf( strLine.c_str(), "%s%d", chksum, &docid );
		iUrl.m_sChecksum = chksum;
		iUrl.m_nDocId = docid;
		vecCUrl.push_back(iUrl);
	}

	int nTemp = 0;
	while (getline(ifsDoc,strLine)){
		int docid,pos,length;
		char chksum[33];

		memset(chksum, 0, 33);
		sscanf( strLine.c_str(), "%d%d%d%s", &docid, &pos, &length,chksum );
	
		iDocument.m_nDocId = docid;
		iDocument.m_nPos = pos;
		iDocument.m_nLength = length;
		iDocument.m_sChecksum = chksum;
		vecCDocument.push_back(iDocument);
	}

/*
	// find page according to a url
	// ===================================
	string c,key;
	cin >> c;
	if( c == "c" ) return(0);


	CMD5 iMD5;
	iMD5.GenerateMD5( (unsigned char*)c.c_str(), c.size() );
	key = iMD5.ToString();

	int low=0, high = vecCUrl.size()-1, mid = 0;

	cout << "url: " << c << " len: " << c.size() << endl;
	cout << "md5: " << key << endl;
	cout << "high: " << high << endl;

	bool bFound = false;
	while (low <= high ){
		mid = (low+high)/2;

		if ( key == vecCUrl[mid].m_sChecksum ){
			docId = vecCUrl[mid].m_nDocId;
			// cout << docId << endl;
			bFound = true;
			break;
		} else if ( key < vecCUrl[mid].m_sChecksum ){
			high = mid -1;
		} else {
			low = mid + 1;
		}
	}

	if (!bFound){ 
		cout << "not found" << endl;
		return(0);
	}

	return(0);
	//==============================
*/


	strFileName += ".seg";
	ofstream fout(strFileName.c_str(), ios::in|ios::out|ios::trunc|ios::binary);
	for ( docId=0; docId<MAX_DOC_ID; docId++ ){

		// find document according to docId
		int length = vecCDocument[docId+1].m_nPos - vecCDocument[docId].m_nPos -1;
		char *pContent = new char[length+1];
		memset(pContent, 0, length+1);
		ifs.seekg(vecCDocument[docId].m_nPos);
		ifs.read(pContent, length);

		char *s;
		s = pContent;

		// skip Head
		int bytesRead = 0,newlines = 0;
		while (newlines != 2 && bytesRead != HEADER_BUF_SIZE-1) {
			if (*s == '\n')
				newlines++;
			else
				newlines = 0;
			s++;
			bytesRead++;
		}
		if (bytesRead == HEADER_BUF_SIZE-1) continue;


		// skip header
		bytesRead = 0,newlines = 0;
		while (newlines != 2 && bytesRead != HEADER_BUF_SIZE-1) {
			if (*s == '\n')
				newlines++;
			else
				newlines = 0;
			s++;
			bytesRead++;
		}
		if (bytesRead == HEADER_BUF_SIZE-1) continue;

		//iDocument.m_sBody = s;
		iDocument.RemoveTags(s);
		iDocument.m_sBodyNoTags = s;

		delete[] pContent;
		string strLine = iDocument.m_sBodyNoTags;

                CStrFun::ReplaceStr(strLine, "&nbsp;", " ");
		CStrFun::EmptyStr(strLine); // set " \t\r\n" to " "


		// segment the document
		CHzSeg iHzSeg;
		strLine = iHzSeg.SegmentSentenceMM(iDict,strLine);
		fout << docId << endl << strLine;
		fout << endl;
		
	}

	return(0);
}
