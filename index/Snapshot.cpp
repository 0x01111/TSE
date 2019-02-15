#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include "Md5.h"
#include "Url.h"
#include "Document.h"
#include "StrFun.h"

using namespace std;

//=====================
#define MaxNameLength 255
#define MaxValueLength 2000
#define MAXFORMITEM 40

void Translate(char* SourceStr);
int GetInputs();
//int PrintResults();

typedef struct {
        char Name[MaxNameLength];
        char Value[MaxValueLength];
} HtmlInput_Struct;

HtmlInput_Struct HtmlInputs[MAXFORMITEM];
int HtmlInputCount=0;

const unsigned HEADER_BUF_SIZE = 1024;

int main(int argc, char* argv[])
{
	string strLine;
	CUrl iUrl;
	vector<CUrl> vecCUrl;
	CDocument iDocument;
	vector<DocIdx> vecDocIdx;
	int docId = -1;

	GetInputs();

	ifstream ifs("./Data/Tianwang.raw.2559638448");
	if (!ifs) {
		cerr << "Cannot open tianwang.img.info for input\n";
                return -1;
        }

	ifstream ifsUrl("./Data/Url.idx.sort_uniq");
        if (!ifsUrl) {
                cerr << "Cannot open Url.idx.sort_uniq for input\n";
                return -1;
        }
	ifstream ifsDoc("./Data/Doc.idx");
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

	while (getline(ifsDoc,strLine)){
		//int docid,pos,length;
		//char chksum[33];
		DocIdx di;

		//memset(chksum, 0, 33);
		//sscanf( strLine.c_str(), "%d%d%d%s", &docid, &pos, &length,chksum );
		sscanf( strLine.c_str(), "%d%d", &di.docid, &di.offset);
		//iDocument.m_nDocId = docid;
		//iDocument.m_nPos = pos;
		//iDocument.m_nLength = length;
		//iDocument.m_sChecksum = chksum;
		vecDocIdx.push_back(di);
	}

	// find page according to a url
	string c,key;
	//cin >> c;



	c = HtmlInputs[1].Value;
	//cout << c << endl;
	//return (0);

	CMD5 iMD5;
	//iMD5.GenerateMD5( (unsigned char*)c.c_str(), c.size() );
	iMD5.GenerateMD5( (unsigned char*)c.c_str(), c.size() );
	key = iMD5.ToString();

	int low=0, high = vecCUrl.size()-1, mid = 0;

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


		// find document according to docId
		int length = vecDocIdx[docId+1].offset - vecDocIdx[docId].offset -1;
		char *pContent = new char[length+1];
		memset(pContent, 0, length+1);
		ifs.seekg(vecDocIdx[docId].offset);
		ifs.read(pContent, length);

		char *s = pContent;

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
		if (bytesRead == HEADER_BUF_SIZE-1) return(0);


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
		if (bytesRead == HEADER_BUF_SIZE-1) return(0);


		string tmp = HtmlInputs[0].Value;
		string::size_type idx;
		vector<string> vecQuery;

		//cout << tmp << "<br>";
		//cout << "tmp.size():" << tmp.size() << "<br>";

		while ( (idx = tmp.find(" ")) != string::npos ) {
			if (tmp.substr(0,idx).empty()) continue;
			vecQuery.push_back(tmp.substr(0,idx));
			tmp = tmp.substr(idx+1);
		}
		vecQuery.push_back(tmp.substr(0,idx));


cout << "<TABLE cellSpacing=0 cellPadding=10 width=100% bgColor=#ffffff border=1 color=#ffffff>"
  << "<TBODY>"
  <<   "<TR>"
  <<     "<TD><FONT color=black size = 2>"
  <<	 "这是</font>"
  <<     "<a href=http://162.105.80.60/yc/TSE/><B><FONT color=#0039b6 size = 2>TSE搜索</a></FONT></B>"
  <<     "<FONT color=black size = 2>提供的网页快照</FONT>"
  <<     "<br><font size=2>您的查询词都已标明如下：";

		for (unsigned int i=0; i<vecQuery.size(); i++){
			if (i%2 == 0 ){
  				cout << "<B style='color:black;background-color:#ffff66'>";
			}else{
  				cout << "<B style='color:black;background-color:#A0FFFF'>";
			}

			cout << "<a href=#n" << CStrFun::itos(i) 
				<< ">" << vecQuery[i] << "</a>&nbsp;</B>";
		}

cout << " (点击查询词，可以跳到它在文中首次出现的位置)" << endl;
cout <<     "<br><font color=gray size = 2>(TSE和网页<a href=\"" << HtmlInputs[1].Value << "\">"
  <<	 "<font color=blue>" << HtmlInputs[1].Value << "</font></a>的作者无关，不对其内容负责。)"
  <<	 "</font></font></td></tr></table>"
  <<     "<BR>"
  <<     "</TD>"
  <<   "</TR>"
  <<   "<TR>"
  <<   "</TR>"
  <<"</TBODY>"
  <<"</TABLE>" << endl
  <<"<meta http-equiv=\"Content-Type\" Content=\"text/html; charset=gb2312\">" << endl;



		string line=s;
		
		for (unsigned int i=0; i<vecQuery.size(); i++){
			string newKey = "<a name=n" + CStrFun::itos(i) + "></a>"
			+ "<b style=\"color:white;background-color: #CE0000\">"
			+ vecQuery[i] + "</b>";

			CStrFun::ReplaceStr(line, vecQuery[i], newKey);
		}

		cout << line << endl;

	return(0);
}


/* 
 * Get form information throught environment varible.
 * return 0 if succeed, otherwise exit.
 */
int GetInputs()
{
        int i,j;
	char *mode = getenv("REQUEST_METHOD");
        char *tempstr;
	//char *in_line;
	char *in_line;
	int length;

	printf("Content-type: text/html\n\n");
	//printf("<html>\n");
	//printf("<head>\n");
	//cout <<"<LINK href=\"style.css\" rel=stylesheet>" << endl;

	if (mode==NULL) return 1;

	if (strcmp(mode, "POST") == 0) {

		length = atoi(getenv("CONTENT_LENGTH"));
		if (length==0 || length>=256)
			return 1;
		in_line = (char*)malloc(length + 1);
		read(STDIN_FILENO, in_line, length);
		in_line[length]='\0';

	} else if (strcmp(mode, "GET") == 0) {
		char* inputstr = getenv("QUERY_STRING");
		length = strlen(inputstr);
		if (inputstr==0 || length>=256)
			return 1;
		in_line = (char*)malloc(length + 1);
		strcpy(in_line, inputstr);
	}


	tempstr = (char*)malloc(length + 1);
	if(tempstr == NULL){
		printf("<title>Error Occurred</title>\n");
		printf("</head><body>\n");
		printf("<p>Major failure #1;please notify the webmaster\n");
		printf("</p></body></html>\n");
		fflush(stdout);
		exit(2);
	}

        j=0;
        for (i=0; i<length; i++)
        {
                if (in_line[i] == '=')
                {
                        tempstr[j]='\0';
                        Translate(tempstr);
                        strcpy(HtmlInputs[HtmlInputCount].Name,tempstr);
                        if (i == length - 1)
                        {
                                strcpy(HtmlInputs[HtmlInputCount].Value,"");
                                HtmlInputCount++;
                        }
                        j=0;
                }
                else if ((in_line[i] == '&') || (i==length-1))
                {
                        if (i==length-1)
                        {
                                if(in_line[i] == '+')tempstr[j]=' ';
                                else tempstr[j] = in_line[i];
                                j++;
                        }
                        tempstr[j]='\0';
                        Translate(tempstr);
                        strcpy(HtmlInputs[HtmlInputCount].Value,tempstr);
                        HtmlInputCount++;
                        j=0;
                } else if (in_line[i] == '+') {
                        tempstr[j]=' ';
                        j++;
                } else {
                        tempstr[j]=in_line[i];
                        j++;
                }
        }

	if(in_line) free(in_line);
	if(tempstr) free(tempstr);

        return 0;
}

//将字符串SourceStr中用十六进制数表示的Ascii 码转换成正常字符^M
void Translate(char* SourceStr)   
{
	int i=0;
	int j=0;
	char *tempstr,tempchar1,tempchar2;

	tempstr = (char*)malloc(strlen(SourceStr) + 1);
	if(tempstr == NULL){
		printf("<title>Error Occurred</title>\n");
		printf("</head><body>\n");
		printf("<p>Major failure #1;please notify the webmaster\n");
		printf("</p></body></html>\n");
		fflush(stdout);
		exit(2);
	}

	while (SourceStr[j])
	{
		if ((tempstr[i]=SourceStr[j])=='%'){
			if (SourceStr[j+1]>='A')
				tempchar1=((SourceStr[j+1]&0xdf)-'A')+10;
			else
				tempchar1=(SourceStr[j+1]-'0');
			if (SourceStr[j+2]>='A')
				tempchar2=((SourceStr[j+2]&0xdf)-'A')+10;
			else
				tempchar2=(SourceStr[j+2]-'0');
				tempstr[i]=tempchar1*16+tempchar2;
			j=j+2;
		}
		i++;
		j++;
	}
	tempstr[i]='\0';
	strcpy(SourceStr,tempstr);

	if(tempstr) free(tempstr);
}

/*
int PrintResults()
{
        printf("<title>提交成功</title>\n");
	printf("<meta http-equiv=\"refresh\" content=\"5;url=http://e.pku.edu.cn/\">\n");
        printf("<center>\n");
	printf("提交成功!<br><br>\n");
	printf("谢谢您的支持<br><br>\n");
	printf("注意: 收录过程不含人工干预，天网不保证收录您提交的网站。<br><br>\n");
	printf("本页 5 秒后自动转到 相关网页 ......<br><br>\n");
	printf("&copy 2003 北大网络实验室.<br><br>\n");
	printf("</center></body>\n<html>");

	return 0;
}
*/
