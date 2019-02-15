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
	while ( (idx = m_sSegQuery.find("/  ")) != string::npos ) { 
		vecTerm.push_back(m_sSegQuery.substr(0,idx)); 
		m_sSegQuery = m_sSegQuery.substr(idx+3); 
	}
}

void CQuery::SetQuery()
{
	string q = HtmlInputs[0].Value;
	CStrFun::Str2Lower(q,q.size());
	m_sQuery = q;
}

void CQuery::SetStart()
{
	m_iStart = atoi(HtmlInputs[1].Value);
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

//LB_c: vecTerm�д洢���û���ѯ�ִ��ִ��Ժ�Ĺؼ��ʣ�mapBucketΪ���ű�setRelevantRst�洢���յĲ�ѯ�Ľ��(����洢
// ���ǽ����ҳ��docid)
bool CQuery::GetRelevantRst(vector<string> &vecTerm, 
	map<string,string> &mapBuckets, 
	set<string> &setRelevantRst) const
{
	//LB_c: ��ʱ�洢�Ѿ���ѯ�Ľ��
	set<string> setSRst;
	bool bFirst=true;

	//LB_c: �ֱ��vecTerm�е�ÿһ���ؼ��ʽ��в�ѯ
	vector<string>::iterator itTerm = vecTerm.begin();
	for ( ; itTerm != vecTerm.end(); ++itTerm ){

		//LB_c: setRelevantRstΪ�Ѳ�ѯ�Ľ������setRelevantRst����setSRst�У����潫�õ���
		setSRst.clear();
		copy(setRelevantRst.begin(), setRelevantRst.end(), 
		inserter(setSRst,setSRst.begin()));

		//LB_c: mapRstDoc��һ��������ʱͳ�Ƶ�map��string��Ӧһ��docID��int�Ǹ�docID���ֵĴ���(Ҳ���ǵ�ǰ�ؼ�����docid
		// ����ҳ�г��ֵĴ�����Ҳ��Ϊ"��Ƶ"�����潫��Ϊ"��Ƶ"), �����Ǹ��ݴ�Ƶֵ������������������(���ؼ��ʳ��ִ���
		// Խ�����ҳӦ��Խ"��")��
		map<string,int> mapRstDoc;
		string docid;
		int doccnt;
		//LB_c: �ڵ��ű��в�ѯ�ؼ���(*itTerm)
		map<string,string>::iterator itBuckets = mapBuckets.find(*itTerm);	
		//LB_c: �ڵ��ű����ҵ��˸ùؼ���
		if (itBuckets != mapBuckets.end()){

			//LB_c: ��ȡ�ùؼ��ʳ��ֵ��ĵ�ID�б�(�����ű��¼�ĵڶ�������˵����ѿ��Կ�����2���е����ļ��Ľṹ)
			string strBucket = (*itBuckets).second;

			string::size_type idx;
			idx = strBucket.find_first_not_of(" ");
			strBucket = strBucket.substr(idx);

			//LB_c: ѭ�����ĵ�ID�б��ַ����л�ȡһ���ĵ�ID���������Ƶ������mapRstDoc��
			while ( (idx = strBucket.find(" ")) != string::npos ) {
				docid = strBucket.substr(0,idx);
				if (docid.empty()) continue;
				doccnt = 0;	//LB_c: �����Ƶ
				//LB_c: ��mapRstDoc�в�ѯ��docid�Ƿ���ֹ�
				map<string,int>::iterator it = mapRstDoc.find(docid);
				//LB_c: ���docid���ֹ�
				if ( it != mapRstDoc.end() ){
				//LB_c: ��ȡ��Ƶ((*it).second)��1����doccnt
				doccnt = (*it).second + 1;
				//LB_c: ��mapRstDoc�Ѹ�����¼ɾ�������潫���²���
				mapRstDoc.erase(it);
				}

				//LB_c: ��������¼���²��뵽mapRstDoc����ʵ��ɾ���ٲ���������¼�Ľ������docid�Ĵ�Ƶ����1
				//LB_c: ����Ӧ���е�����! ���docidû���ֹ�����ôdoccnt��ֵΪ0������뵽mapRstDoc�Ķ�Ӧ��docid
				// �Ĵ�Ƶ0������ǰ��doccnt�ĳ�ֵ�ǲ���Ӧ��Ϊ1��?
				mapRstDoc.insert( pair<string,int>(docid,doccnt) );

				//LB_c: ȥ����������docid����strBucket������������һ���ĵ�ID
				strBucket = strBucket.substr(idx+1);
			}

			//LB_c: �����ⲿ���Ǵ���strBucket�����һ��docid����Ϊwhileѭ������ʱ�����һ��docid��û�д���
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
		//LB_c: ��һ���ִ������mapRstDoc�洢����һϵ��docid�͸�docid�Ĵ�Ƶ��

		// sort by term frequencty
		//LB_c: �ⲿ���ǶԸղŵĴ��д�Ƶ���ĵ���ѯ���mapRstDoc�������������������뵽newRstDoc�С�
		//LB_c: ע��newRstDoc�����ͣ���һ����Ϊint��ʾdocid�Ĵ�Ƶ���ڶ�������string��ʾdocid����������
		// ���������----�Լ�ֵ(��Ƶ)�Ľ������У�ע��newRstDoc��multimap��Ҳ���Ǽ�ֵ�����ظ���
		multimap<int, string, greater<int> > newRstDoc;
		map<string,int>::iterator it0 = mapRstDoc.begin();
		for ( ; it0 != mapRstDoc.end(); ++it0 ){
			newRstDoc.insert( pair<int,string>((*it0).second,(*it0).first) );
		}

		//LB_c: �ⲿ���ǽ���ǰ�ؼ���(*itTerm)�������ѯ���newRstDoc���뵽���յĲ�ѯ���setRelevantRst�У�
		// ����Ҫ�ο�ǰ��Ĺؼ��ʲ�ѯ�����
		multimap<int,string>::iterator itNewRstDoc = newRstDoc.begin();	
		//LB_c: �����յĲ�ѯ���setRelevantRst���
		setRelevantRst.clear();	
		//LB_c: ѭ����ȡnewRstDoc�е�ÿһ����¼(��Щ��¼�ǰ�docid�Ĵ�Ƶ�����)������������뵽���ս����
		for ( ; itNewRstDoc != newRstDoc.end(); ++itNewRstDoc ){

			//LB_c: ��ȡ������¼��docid
			string docid = (*itNewRstDoc).second;		
			//LB_c: �����ǰ�ؼ����ǵ�һ����ѯ�Ĺؼ��ʣ���ֱ�Ӳ��뵽�������
			if (bFirst==true) {
				setRelevantRst.insert(docid);
				continue;
			}

			//LB_c: ������ǵ�һ���ؼ��ʲ�ѯ�����Ѳ�ѯ�����setSRst���Ƿ��и�docid��Ҳ����ǰ���ѯ�Ĺؼ���
			// ��û�г�����docid����ҳ�С�����Ҳ������TSE�����Ĺ���: ֻ�����йؼ��ʶ����ֵ���ҳ����Ч�����ֹؼ�
			// �ʳ��ֵ���ҳ����Ϊ������������setSRst���и�docid˵��docid����ҳҲ����ǰ���ѯ�Ĺؼ���, ��
			// docid���뵽���ս����setRelevantRst�С�
			if ( setSRst.find(docid) != setSRst.end() ){	
				setRelevantRst.insert(docid);
			}
		}

		//LB_c: ����˼��һ��! ���Ƚ�setRelevantRst��գ�Ȼ�󽫵�ǰ�ؼ���(*itTerm)��������newRstDoc���뵽���ս����
		// setRelevantRst�С�Ҳ�������յĽ�������������һ����ѯ�ؼ��ʵĴ�Ƶ����ģ������һ���ؼ��ʳ��ִ��������ҳ
		// ����ǰ�档

		bFirst = false;
	}

	return true;
}

