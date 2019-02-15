#include "Comm.h"
#include "DisplayRst.h"
#include "StrFun.h"


using namespace std;

CDisplayRst::CDisplayRst()
{
}

CDisplayRst::~CDisplayRst()
{
}

//LB_c: strQueryΪԭʼ���û���ѯ����fUsedMsecΪ������ʱ��iRstNumΪ�����������������startΪ��ʾ������ĵڼ�ҳ
bool CDisplayRst::ShowMiddle(string strQuery, float fUsedMsec, unsigned iRstNum, unsigned start)
{
	//LB_c: iPageNumΪ�����ҳ����RstPerPageΪÿҳ��ʾ����������һ������
	unsigned iPageNum = 0;
	if (iRstNum%RstPerPage == 0){
		iPageNum = iRstNum/RstPerPage;
	} else {
		iPageNum = iRstNum/RstPerPage + 1;
	}

	//LB_c: ��ʾ��ʾ��Ϣ: �û���ѯ�Ĵ���������ʱ�����ж������������ǰ��ʾ����x��y��
	cout << "<title>TSE Search</title>\n"
		<< "<font  color=#008080 size=2>" << endl
		<< "����: <b><font color=\"#000000\" size=\"2\">" 
		<< strQuery << "</b></font>" << endl
		<< "��ʱ<b><font color=\"#000000\" size=\"2\">"
		<< fUsedMsec
		<< "</font></b> ����,���ҵ�<b><font color=\"#000000\" size=\"2\">"
		<< iRstNum
		<< "</font></b> ƪ�ĵ�,�����ǵ� <b><font color=\"#000000\" size=\"2\">";
	if (iRstNum == 0){
		cout << "0</font></b>���� <b><font color=\"#000000\" size=\"2\">"
			<< "0</font></b>��<br>" << endl;
		return true;
	}
	cout << (start-1)*RstPerPage + 1 << "</font></b>���� <b><font color=\"#000000\" size=\"2\">";
	if (iRstNum >= start*RstPerPage ) {
		cout << start*RstPerPage << "</font></b>��&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
	} else {
		cout << iRstNum << "</font></b>��&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
	}

	//LB_c: ���û�ѡ��Ľ��ҳ�����ӣ�
	cout << "ѡ��ҳ��: " ;	
	for (unsigned i=0; i<iPageNum; i++) {
		//LB_c: ��ǰҳ���ҳ��û������
		if (i+1 == start ) {
			cout << i+1 << "</a>&nbsp;&nbsp;";
		}
		//LB_c: ����ҳ���ṩ���ӣ�ע��������"/yc-cgi-bin/index/TSESearch?word=***&start=***"���������˵��
		else{
			cout << "<a href=\"/yc-cgi-bin/index/TSESearch?word=" 
				<< strQuery << "&start=" << i+1 << "\">"
				<< i+1 << "</a>&nbsp;&nbsp;";
		}
	}
	
	return true;
}
//��6�����ᵽ��������������ҳ���н����µ���������ʾ����������ģ�Ϊʲô��? ���������˵����ҳ������Ϊ
//"yc-cgi-bin/index/TSESearch?word=***&start=***"��ʽ����Ȼ�ڶ�����ֵ����start������HtmlInputs[1].Value����
//�û������ҳ�ţ�����m_iStart��ֵҲ����ȷ�ģ������ʾ����������ġ�
//��ҳ�����ӿ��Կ������û����ҳ���Ժ���ִ����һ��cgi����/yc-cgi-bin/index/TSESearch����������������һ�Σ�������
//ֱ�����ϴεĽ������ȡ����Ӧҳ�����ݽ�����ʾ������Ϊ����? ���ƺ�̫�������ˣ�������������Ѿ����ˣ�ֻ��򵥵�
//ʵ����ʾ��ͬ��ҳ�����ˡ�

bool CDisplayRst::ShowBelow(vector<string>&vecQuery, set<string> &setRelevantRst, 
		vector<DocIdx> &vecDocIdx, unsigned start)
{
	cout << "<ol>" << endl;

	set<string>::iterator it= setRelevantRst.begin();
	unsigned iDocNumber=0;
	//LB_c: startΪ�û�ѡ�����ʾ�������ҳ�ţ�RstPerPageΪÿҳ��ʾ�ļ�¼�����������������Ҫ��ʾ�Ľ����ֹ��ţ�
	// ����ʾiRstBegin��iRstEnd�Ľ����¼������Ҳ���Կ���startҳ��Ӧ���Ǵ�1��ʼ�ġ�
	unsigned iRstBegin = (start-1)*RstPerPage;
	unsigned iRstEnd = start*RstPerPage - 1;

	vector<string> vecRefUrl;
	vector<string>::iterator itVecRefUrl;
	cout << "<tr bgcolor=#e7eefc>";
	bool bColor = true;

	//LB_c: ��ԭʼ��ԭʼ��ҳ���ݿ⣬�û����"��ҳ����"ʱҪ���ж�����ҳ����ʾ����������Ҳ˵����ҳ������
	//���ڷ���������ʷ���ݣ������Ǵ���ַ�õ���ʵʱ��ҳ��
	ifstream ifs(RAWPAGE_FILE_NAME.c_str());
	if (!ifs) {
		cout << "Cannot open " << RAWPAGE_FILE_NAME << " for input\n";
		return false;
	}

	for ( ; it!=setRelevantRst.end(); ++it,iDocNumber++ ){
		//LB_c: �������ж���ţ���setRelevantRst��ȡ����iRstBegin����iRstEnd����¼��
		if (iDocNumber < iRstBegin ) continue;
		if (iDocNumber > iRstEnd ) break;

		cout << "<li><font color=black size=2>" << endl ;
		//LB_c: ��ȡ�����¼��docid
		int docId = atoi( (*it).c_str() );
		//LB_c: vecDocIdx��main������˵����������ҳ������(��¼docid��offset��ӳ��)�������ȡǰ��������ҳ��
		//ԭʼ��ҳ���ݿ��е�offset������õ�����ҳ�ĳ��ȡ�
		int length = vecDocIdx[docId+1].offset - vecDocIdx[docId].offset;

		//LB_c: ����������pContent����ԭʼ��ҳ���ݿ��ļ��ж�������ҳ����
		char *pContent = new char[length+1];
		memset(pContent, 0, length+1);
		ifs.seekg(vecDocIdx[docId].offset);
		ifs.read(pContent, length);

		char *s;
		s = pContent;
		string url,tmp = pContent;
		string::size_type idx1 = 0, idx2=0;

		//LB_c: ����ҳ�����а�url��ȡ����
		idx1 = tmp.find("url: ");
		if( idx1 == string::npos ) continue;
		idx2 = tmp.find("\n", idx1);
		if( idx1 == string::npos ) continue;
		url = tmp.substr(idx1+5, idx2 - idx1 - 5);

		//LB_c: vecQuery��main�����н��ܹ������������ָ��Ժ�Ĺؼ��ʣ����ｫ��Щ�ؼ�����"+"��������
		//����ҳ��������ʾ��������ʾ�û���
		string word;
		for(unsigned int i=0; i< vecQuery.size(); i++){ 
			word = word + "+" + vecQuery[i]; 
		}
		word = word.substr(1);

		//========================================================================================================
		//LB_c: �������ÿ�������¼�ľ������ݣ�����: ��ҳ�����ӣ���ҳ���ȣ���ҳ�������Ӻ���ҳ����ժҪ

		//LB_c: ��ҳ�������ӵ���һ��cgi����: /yc-cgi-bin/index/Snapshot�������"��ҳ����"����cgi����
		///yc-cgi-bin/index/Snapshot������
		cout << "<a href=" << url << ">" << url << "</a>,&nbsp;"
			<< length << "<font  color=#008080>�ֽ�</font>" << ",&nbsp;"
			<< "<a href=/yc-cgi-bin/index/Snapshot?"
			<< "word=" << word << "&"
			<< "url="<< url
			<< " target=_blank>"
			<< "[��ҳ����]</a>" 
			<< endl << "<br>";

		if (length > 400*1024) {    // if more than 400KB
			delete[] pContent;
			continue;
		}

		//LB_c: �����Ǵ���ҳ��������ȡ���ģ�Ȼ�����������ȡ��ҳժҪ����������ʾ�����ﲻ��ϸ���͡�
		// skip HEAD 
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

		CDocument iDocument;
		iDocument.RemoveTags(s);
		iDocument.m_sBodyNoTags = s;
		delete[] pContent;
		string line = iDocument.m_sBodyNoTags;
		CStrFun::ReplaceStr(line, "&nbsp;", " ");
		CStrFun::EmptyStr(line); // set " \t\r\n" to " "

		// abstract
		string reserve;
		if ((unsigned char)line.at(48) < 0x80) {
			reserve = line.substr(0,48);
		}else{
			reserve = line.substr(0,48+1);
		}
		reserve = "[" + reserve + "]";
		unsigned int resNum = 128;
		if (vecQuery.size() == 1) resNum = 256;
		for(unsigned int i=0; i< vecQuery.size(); i++){
			string::size_type idx = 0, cur_idx;
			idx = line.find(vecQuery[i],idx);
			if (idx == string::npos) continue;
			if (idx > resNum ) {
				cur_idx = idx - resNum;
				while ((unsigned char)line.at(cur_idx) > 0x80 && cur_idx!=idx) { 
					cur_idx ++; 
				}
				reserve += line.substr(cur_idx+1, resNum*2);
			}else{
				reserve += line.substr(idx, resNum*2);
			}
			reserve += "...";
			// highlight
			string newKey = "<font color=#e10900>" + vecQuery[i] + "</font>";
			CStrFun::ReplaceStr(reserve, vecQuery[i], newKey);
		}
		line = reserve;
		cout << line << endl << endl;
		//========================================================================================================		
	}

	cout << "</ol>";
	cout << "<br><br><hr><br>";
	cout << "&copy 2004 ��������ʵ����<br><br>\n";
	cout << "</center></body>\n<html>";

	return true;
}

//������ҳ���չ��ܵ�ʵ����Snapshot.cpp�У���ϵ�������в�չ��������ϸ���͡�������һ��������ָ��һ�£���Snapshot.cpp
//Դ�����е�֪�����չ��ܴ����cgi������ݴ������ҳurl��ԭʼ��ҳ���ݿ��ж�����ҳ������ʾ��������������ҳ����Ҳ�����
//�ܸ��ӣ��ȼ���url�����ļ����ٸ��ݴ���url��MD5ֵ���ļ����ҳ���Ӧ��docid��Ȼ���ԭʼ��ҳ���ݿ����ҵ�����ҳ��������
//������ʾ��Ϊ������������? ��ShowBelow�в����Ѿ��õ������ҳ����ҳ�������𣬿��Ի�����������Ҫ��ʾ��ҳ����ʱֱ��
//ȡ��������ʾ���Ϳ�������?


bool CDisplayRst::ShowTop()
{
	string strHost = string(getenv("HTTP_HOST"));
	//LB_c: �����ҳ��body��ǩ
	cout << "<body bgcolor=#ffffff topmargin=2 marginheight=2>"
	<< "<table class=border=0 width=100% cellspacing=0 cellpadding=0 height=29>" << endl
	<< "<tr>" << endl
	//LB_c: ���Ͻǵ�"��������"��logoͼƬ������ͼƬ����һ����strHost/yc/TSE/������(strHost����վrootĿ¼����apache���õ���վĿ¼��
	//ǰ���Ѿ�����Ϊ/var/www/html/)�����������logo���strHost/yc/TSE/�е�index.html������ҳ��strHost�е�index.htmlʵ������һ���ģ�
	//Ҳ����������ҳ����֪��Ϊʲô��ֱ�����ӵ�strHost�е�index.html?
	<< "<td width=36% rowspan=2 height=1>" << "<a href=http://" 
	<< strHost << "/yc/TSE/><img border=0 src=/yc/TSE/tsetitle.JPG width=308 height=65></a></td>" << endl
	//LB_c: ������"������ҳ"��"ʹ�ð���"���ӣ�ǰ���������logo����һ�����������ӵ�һ�������ϵİ����ֲᡣ
	<< "<td width=64% height=33 ><font size=2><a href=http://" 
	<< strHost << "/yc/TSE/>������ҳ</a>| <a href=http://e.pku.edu.cn/gbhelp.htm>ʹ�ð���</a> </font><br></td>" << endl
	<< "</tr>" << endl;

	//LB_c: ��������ҳ����������һ���²�ѯ��form������������������ť����������ʾҳ���ֵ��
	cout << "<tr>" << endl
	<< "<td><p align=\"left\">" << endl
	<< "<form method=\"get\" action=\"/yc-cgi-bin/index/TSESearch\" name=\"tw\">" << endl
	//LB_c: ���������
	<< "<input type=\"text\" name=\"word\" size=\"55\">" << endl
	//LB_c: �²�ѯ��ť��ע���input��ǩ��û������name
	<< "<INPUT TYPE=\"submit\" VALUE=\" �²�ѯ \">&nbsp;" << endl
	//LB_c: ���ӵļ�ֵ�ԣ�����Ϊ"start"��ֵΪ1��������ָʾ��ʾ��������ĵ�1ҳ��
	<< "<input type=\"hidden\" name=\"start\" value=\"1\">" << endl
	<< "</form>" << endl
	<< "</tr>" << endl
	<< "</table>" << endl;

	//LB_c: �м���ɫ�ĺ���������д��"ͼƬ"
	cout << "<table border=0 width=100% cellspacing=1 cellpadding=0 height=1>" << endl
	<< "<tr>" << endl
	<< "<td width=68 align=center bgcolor=#000066 valign=middle><font size=2><b><font color=#FFFFFF>ͼ  Ƭ</font></b></font></td>"
 	<< endl
	<< "</tr>" << endl
	<< "<tr>"
	<< "<td width=100% align=left colspan=3 height=0>"
	<< "</td></tr>" << endl
	<< "</table>" << endl;

	return true;
}
//��6�����ᵽ��������������ҳ���н����µ���������ʾ����������ģ�Ϊʲô��? ��һ�´����е��²�ѯ��form��form����
//����input����ֻ������input��name�������û����"�²�ѯ"��ť���ύ��URL����������ֵ��(word��start)�������ѯ"������ѧ"�õ�
//http://localhost:8080/yc-cgi-bin/index/TSESearch?word=%B1%B1%BE%A9%B4%F3%D1%A7&start=1�����HtmlInputs[1]�Ͷ�Ӧstart��ֵ�ԣ�
//HtmlInputs[1].Value��ֵΪ1������m_iStart����Ϊ1������ʾ������ĵ�1ҳ��
//���������"�²�ѯ"��ť��input��ǩ�ж�����name����URL����������ֵ�ԣ��ڶ�����ֵ�Զ�Ӧ"�²�ѯ"��ť��HtmlInputs[1]�Ͳ���start��ֵ���ˣ�
//����Ҳ�����! ���������ǿ��Բ���һ�¡�