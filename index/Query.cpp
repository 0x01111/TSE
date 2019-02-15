// Query handling
#include <unistd.h>

#include <fstream>
#include <algorithm>

#include "Query.h"
#include "Comm.h"
#include "StrFun.h"

using namespace std;


CQuery::CQuery()
{
	m_sQuery = "";
	m_sSegQuery = "";
	m_iStart = 0;
	HtmlInputCount=0;
}

CQuery::~CQuery()
{
}

void CQuery::ParseQuery(vector<string> &vecTerm)
{
	string::size_type idx; 
    if (1==1) {
        vecTerm.push_back(m_sQuery);
        return;
    }
	while ( (idx = m_sSegQuery.find("/  ")) != string::npos ) { 
		vecTerm.push_back(m_sSegQuery.substr(0,idx)); 
		m_sSegQuery = m_sSegQuery.substr(idx+3); 
	}
}

void CQuery::SetQuery(string & q)
{
    CStrFun::Str2Lower(q,q.size());
    m_sQuery = q;
}

void CQuery::SetQuery()
{
	//string q = HtmlInputs[0].Value;
	string q = HtmlInputs[0].Value;
	CStrFun::Str2Lower(q,q.size());
	m_sQuery = q;
}

void CQuery::SetStart()
{
	//m_iStart = atoi(HtmlInputs[1].Value);
	m_iStart = 1; //atoi(HtmlInputs[1].Value);
}

bool CQuery::GetInvLists(map<string, string> &mapBuckets) const
{
	ifstream ifsInvInfo(INF_INFO_NAME.c_str(), ios::binary);
	if (!ifsInvInfo) {
		cerr << "Cannot open " << INF_INFO_NAME << " for input\n";
		return false;
	}

	string strLine, strWord, strDocNum;
	while (getline(ifsInvInfo, strLine)) {
		string::size_type idx;
		string tmp;


		idx = strLine.find("\t");
		strWord = strLine.substr(0,idx);
		strDocNum = strLine.substr(idx+1);

		mapBuckets.insert(map<string,string>::value_type (strWord, strDocNum));

	}

	return true;
}

/*
bool CQuery::GetDocidUrl(map<string, string> &mapDocidUrl) const
{

	ifstream ifsImgInfo(IMG_INFO_NAME.c_str(), ios::binary); 
	if (!ifsImgInfo) { 
		cerr << "Cannot open " << IMG_INFO_NAME << " for input\n"; 
		return false; 
	} 

	string strLine, strDocid, strUrl; 
	while (getline(ifsImgInfo, strLine)) { 
		string::size_type idx; 
		string tmp; 

		idx = strLine.find("\t"); 
		strDocid = strLine.substr(0,idx); 
		strUrl = strLine.substr(idx+1); 
		mapDocidUrl.insert(map<string,string>::value_type (strDocid, strUrl)); 
	} 

	return true;
}
*/

bool CQuery::GetDocIdx(vector<DocIdx> &vecDocIdx) const
{
	ifstream ifs(DOC_IDX_NAME.c_str(), ios::binary); 
	if (!ifs) { 
		cerr << "Cannot open " << DOC_IDX_NAME << " for input\n"; 
		return false; 
	} 

	string strLine, strDocid, strUrl; 
	while (getline(ifs,strLine)){
		DocIdx di;

		sscanf( strLine.c_str(), "%d%d", &di.docid, &di.offset );
		vecDocIdx.push_back(di);
	}

	return true;
}

/* 
 * Get form information throught environment varible.
 * return 0 if succeed, otherwise exit.
 */
int CQuery::GetInputs()
{
    cout << "get input" << endl;
    if (1 == 1) {
        return 0;
    }
        int i,j;
	char *mode = getenv("REQUEST_METHOD");
        char *tempstr;
	char *in_line;
	int length;

	cout << "Content-type: text/html\n\n";
	//cout << "Cache-Control: no-cache\n";
	//cout << "Expires: Tue, 08 Apr 1997 17:20:00 GMT\n";
	//cout << "Expires: 0\n";
	//cout << "Pragma: no-cache\n\n";

	cout << "<html>\n";
	cout << "<head>\n";
	//cout << "<META HTTP-EQUIV=Pragma CONTENT=no-cache>\n";
	//cout << "<META HTTP-EQUIV=Cache-Control CONTENT=no-cache>\n";
	//cout << "<META HTTP-EQUIV=Expires CONTENT=0>\n";
	cout << "</head>\n";
	cout.flush();
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
			CStrFun::Translate(tempstr);
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
			CStrFun::Translate(tempstr);
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

bool CQuery::RecordQueryLog() const
{
	FILE *fp; 
	time_t current_time; 
	char tmpbuf[20];
	unsigned j;

	fp=fopen("/home/webg/public_html/data/imgsearch_submit.list","a");
	if(fp==NULL){ 
		printf("Content-type:text/html\n\nopen error.\n"); 
		return false; 
	}

	current_time = time(NULL); 
	memset(tmpbuf,0,20); 
	sprintf(tmpbuf,"%s", ctime(&current_time)); 

	for (j=0;j<strlen(tmpbuf);j++) { 
		if(tmpbuf[j]==' ') tmpbuf[j]='-'; 
	} 
	tmpbuf[j-1]=0; 

	fprintf(fp,"%s ",tmpbuf); 
	fprintf(fp,"%s ",m_sQuery.c_str()); 
	fprintf(fp,"%s",";;;\n"); 
	fclose(fp);

	return true;
}

//LB_c: vecTerm中存储的用户查询字串分词以后的关键词，mapBucket为倒排表，setRelevantRst存储最终的查询的结果(里面存储
// 的是结果网页的docid)
bool CQuery::GetRelevantRst(vector<string> &vecTerm, 
	map<string,string> &mapBuckets, 
	set<string> &setRelevantRst) const
{
	//LB_c: 临时存储已经查询的结果
    cout<< "start GetRelevantRst" << endl;
	set<string> setSRst;
	bool bFirst=true;

	//LB_c: 分别对vecTerm中的每一个关键词进行查询
	vector<string>::iterator itTerm = vecTerm.begin();
	for ( ; itTerm != vecTerm.end(); ++itTerm ){

		//LB_c: setRelevantRst为已查询的结果，将setRelevantRst存入setSRst中，后面将用到。
		setSRst.clear();
		copy(setRelevantRst.begin(), setRelevantRst.end(), 
		inserter(setSRst,setSRst.begin()));

		//LB_c: mapRstDoc是一个用于临时统计的map，string对应一个docID，int是该docID出现的次数(也就是当前关键词在docid
		// 的网页中出现的次数，也成为"词频"，后面将称为"词频"), 后面是根据词频值对搜索结果进行排序的(即关键词出现次数
		// 越多的网页应该越"优")。
		map<string,int> mapRstDoc;
		string docid;
		int doccnt;
		//LB_c: 在倒排表中查询关键词(*itTerm)
		map<string,string>::iterator itBuckets = mapBuckets.find(*itTerm);	
        cout <<"term:" << *itTerm << endl;
		//LB_c: 在倒排表中找到了该关键词
		if (itBuckets != mapBuckets.end()){

			//LB_c: 获取该关键词出现的文档ID列表(即倒排表记录的第二项，忘记了的朋友可以看看第2节中倒排文件的结构)
			string strBucket = (*itBuckets).second;

			string::size_type idx;
			idx = strBucket.find_first_not_of(" ");
			strBucket = strBucket.substr(idx);

			//LB_c: 循环从文档ID列表字符串中获取一个文档ID，并计算词频，插入mapRstDoc中
			while ( (idx = strBucket.find(" ")) != string::npos ) {
				docid = strBucket.substr(0,idx);
				if (docid.empty()) continue;
				doccnt = 0;	//LB_c: 计算词频
				//LB_c: 到mapRstDoc中查询该docid是否出现过
				map<string,int>::iterator it = mapRstDoc.find(docid);
				//LB_c: 如果docid出现过
				if ( it != mapRstDoc.end() ){
				//LB_c: 获取词频((*it).second)加1存入doccnt
				doccnt = (*it).second + 1;
				//LB_c: 从mapRstDoc把该条记录删除，下面将重新插入
				mapRstDoc.erase(it);
				}

				//LB_c: 将该条记录重新插入到mapRstDoc，其实先删除再插入这条记录的结果就是docid的词频加了1
				//LB_c: 这里应该有点问题! 如果docid没出现过，那么doccnt的值为0，则插入到mapRstDoc的对应于docid
				// 的词频0，所以前面doccnt的初值是不是应该为1呢?
				mapRstDoc.insert( pair<string,int>(docid,doccnt) );

				//LB_c: 去掉分析过的docid更新strBucket，继续分析下一个文档ID
				strBucket = strBucket.substr(idx+1);
			}

			//LB_c: 下面这部分是处理strBucket中最后一个docid，因为while循环结束时，最后一个docid还没有处理
			// remember the last one
			docid = strBucket;
			doccnt = 0;
			map<string,int>::iterator it = mapRstDoc.find(docid);
			if ( it != mapRstDoc.end() ){
			doccnt = (*it).second + 1;
			mapRstDoc.erase(it);
			}
			mapRstDoc.insert( pair<string,int>(docid,doccnt) );
		}
		//LB_c: 这一部分处理完后，mapRstDoc存储的是一系列docid和该docid的词频。

		// sort by term frequencty
		//LB_c: 这部分是对刚才的带有词频的文档查询结果mapRstDoc进行了排序，排序结果存入到newRstDoc中。
		//LB_c: 注意newRstDoc的类型，第一个域为int表示docid的词频，第二个域是string表示docid，第三个域
		// 是排序规则----以键值(词频)的降序排列，注意newRstDoc是multimap，也就是键值可以重复。
		multimap<int, string, greater<int> > newRstDoc;
		map<string,int>::iterator it0 = mapRstDoc.begin();
		for ( ; it0 != mapRstDoc.end(); ++it0 ){
			newRstDoc.insert( pair<int,string>((*it0).second,(*it0).first) );
		}

		//LB_c: 这部分是将当前关键词(*itTerm)的排序查询结果newRstDoc插入到最终的查询结果setRelevantRst中，
		// 这里要参考前面的关键词查询结果。
		multimap<int,string>::iterator itNewRstDoc = newRstDoc.begin();	
		//LB_c: 将最终的查询结果setRelevantRst清空
		setRelevantRst.clear();	
		//LB_c: 循环读取newRstDoc中的每一条记录(这些记录是按docid的词频排序的)，根据情况插入到最终结果中
		for ( ; itNewRstDoc != newRstDoc.end(); ++itNewRstDoc ){

			//LB_c: 获取该条记录的docid
			string docid = (*itNewRstDoc).second;		
			//LB_c: 如果当前关键词是第一个查询的关键词，则直接插入到结果集中
			if (bFirst==true) {
				setRelevantRst.insert(docid);
				continue;
			}

			//LB_c: 如果不是第一个关键词查询，则看已查询结果集setSRst中是否有该docid，也就是前面查询的关键词
			// 有没有出现在docid的网页中。这里也体现了TSE搜索的规则: 只有所有关键词都出现的网页才有效，部分关键
			// 词出现的网页不作为搜索结果。如果setSRst中有该docid说明docid的网页也包含前面查询的关键词, 则将
			// docid插入到最终结果集setRelevantRst中。
			if ( setSRst.find(docid) != setSRst.end() ){	
				setRelevantRst.insert(docid);
			}
		}

		//LB_c: 这里思考一下! 首先将setRelevantRst清空，然后将当前关键词(*itTerm)的排序结果newRstDoc插入到最终结果集
		// setRelevantRst中。也就是最终的结果排序是以最后一个查询关键词的词频排序的，即最后一个关键词出现次数多的网页
		// 排在前面。

		bFirst = false;
	}

	return true;
}

