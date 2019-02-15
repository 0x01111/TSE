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

//LB_c: strQuery为原始的用户查询串，fUsedMsec为搜索耗时，iRstNum为搜索结果的总条数，start为显示结果集的第几页
bool CDisplayRst::ShowMiddle(string strQuery, float fUsedMsec, unsigned iRstNum, unsigned start)
{
	//LB_c: iPageNum为结果总页数，RstPerPage为每页显示的条数，是一个常量
	unsigned iPageNum = 0;
	if (iRstNum % RstPerPage == 0)
	{
		iPageNum = iRstNum / RstPerPage;
	}
	else
	{
		iPageNum = iRstNum / RstPerPage + 1;
	}

	//LB_c: 显示提示信息: 用户查询的串，搜索耗时，共有多少条结果，当前显示的是x到y条
	cout << "<title>TSE Search</title>\n"
		 << "<font  color=#008080 size=2>" << endl
		 << "查找: <b><font color=\"#000000\" size=\"2\">"
		 << strQuery << "</b></font>" << endl
		 << "费时<b><font color=\"#000000\" size=\"2\">"
		 << fUsedMsec
		 << "</font></b> 毫秒,共找到<b><font color=\"#000000\" size=\"2\">"
		 << iRstNum
		 << "</font></b> 篇文档,下面是第 <b><font color=\"#000000\" size=\"2\">";
	if (iRstNum == 0)
	{
		cout << "0</font></b>到第 <b><font color=\"#000000\" size=\"2\">"
			 << "0</font></b>个<br>" << endl;
		return true;
	}
	cout << (start - 1) * RstPerPage + 1 << "</font></b>到第 <b><font color=\"#000000\" size=\"2\">";
	if (iRstNum >= start * RstPerPage)
	{
		cout << start * RstPerPage << "</font></b>个&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
	}
	else
	{
		cout << iRstNum << "</font></b>个&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
	}

	//LB_c: 供用户选择的结果页的链接，
	cout << "选择页面: ";
	for (unsigned i = 0; i < iPageNum; i++)
	{
		//LB_c: 当前页面的页号没有链接
		if (i + 1 == start)
		{
			cout << i + 1 << "</a>&nbsp;&nbsp;";
		}
		//LB_c: 其他页号提供链接，注意链接是"/yc-cgi-bin/index/TSESearch?word=***&start=***"，后面进行说明
		else
		{
			cout << "<a href=\"/yc-cgi-bin/index/TSESearch?word="
				 << strQuery << "&start=" << i + 1 << "\">"
				 << i + 1 << "</a>&nbsp;&nbsp;";
		}
	}

	return true;
}
//第6节中提到过如果从搜索结果页面中进行新的搜索，显示结果是正常的，为什么呢? 上面代码中说明了页号链接为
//"yc-cgi-bin/index/TSESearch?word=***&start=***"形式，显然第二个键值对是start，所以HtmlInputs[1].Value就是
//用户点击的页号，所以m_iStart的值也是正确的，因此显示结果是正常的。
//由页号链接可以看出，用户点击页号以后又执行了一次cgi程序/yc-cgi-bin/index/TSESearch，即又重新搜索了一次，而不是
//直接在上次的结果集中取出对应页的内容进行显示，这是为何呢? 这似乎太不合理了，搜索结果集都已经有了，只需简单的
//实现显示不同的页就行了。

bool CDisplayRst::ShowBelow(vector<string> &vecQuery, set<string> &setRelevantRst,
							vector<DocIdx> &vecDocIdx, unsigned start)
{
	cout << "<ol>" << endl;

	set<string>::iterator it = setRelevantRst.begin();
	unsigned iDocNumber = 0;
	//LB_c: start为用户选择的显示结果集的页号，RstPerPage为每页显示的记录条数，所以这里计算要显示的结果起止序号，
	// 即显示iRstBegin到iRstEnd的结果记录。这里也可以看出start页号应该是从1开始的。
	unsigned iRstBegin = (start - 1) * RstPerPage;
	unsigned iRstEnd = start * RstPerPage - 1;

	vector<string> vecRefUrl;
	vector<string>::iterator itVecRefUrl;
	cout << "<tr bgcolor=#e7eefc>";
	bool bColor = true;

	//LB_c: 打开原始的原始网页数据库，用户点击"网页快照"时要从中读出网页并显示出来，这里也说明网页快照是
	//存在服务器的历史数据，而不是打开网址得到的实时网页。
	ifstream ifs(RAWPAGE_FILE_NAME.c_str());
	if (!ifs)
	{
		cout << "Cannot open " << RAWPAGE_FILE_NAME << " for input\n";
		return false;
	}

	for (; it != setRelevantRst.end(); ++it, iDocNumber++)
	{
		//LB_c: 这两行判断序号，在setRelevantRst中取出第iRstBegin到第iRstEnd条记录。
		if (iDocNumber < iRstBegin)
			continue;
		if (iDocNumber > iRstEnd)
			break;

		cout << "<li><font color=black size=2>" << endl;
		//LB_c: 获取结果记录的docid
		int docId = atoi((*it).c_str());
		//LB_c: vecDocIdx在main函数中说明过，是网页索引表(记录docid到offset的映射)，这里获取前后两个网页在
		//原始网页数据库中的offset，相减得到该网页的长度。
		int length = vecDocIdx[docId + 1].offset - vecDocIdx[docId].offset;

		//LB_c: 建立缓冲区pContent，从原始网页数据库文件中读出该网页数据
		char *pContent = new char[length + 1];
		memset(pContent, 0, length + 1);
		ifs.seekg(vecDocIdx[docId].offset);
		ifs.read(pContent, length);

		char *s;
		s = pContent;
		string url, tmp = pContent;
		string::size_type idx1 = 0, idx2 = 0;

		//LB_c: 从网页数据中把url提取出来
		idx1 = tmp.find("url: ");
		if (idx1 == string::npos)
			continue;
		idx2 = tmp.find("\n", idx1);
		if (idx1 == string::npos)
			continue;
		url = tmp.substr(idx1 + 5, idx2 - idx1 - 5);

		//LB_c: vecQuery在main函数中介绍过，是搜索串分割以后的关键词，这里将这些关键词用"+"连接起来
		//在网页快照中显示，用于提示用户。
		string word;
		for (unsigned int i = 0; i < vecQuery.size(); i++)
		{
			word = word + "+" + vecQuery[i];
		}
		word = word.substr(1);

		//========================================================================================================
		//LB_c: 以下输出每条结果记录的具体内容，包括: 网页的链接，网页长度，网页快照链接和网页内容摘要

		//LB_c: 网页快照链接到另一个cgi程序: /yc-cgi-bin/index/Snapshot，即点击"网页快照"后，由cgi程序
		///yc-cgi-bin/index/Snapshot来处理。
		cout << "<a href=" << url << ">" << url << "</a>,&nbsp;"
			 << length << "<font  color=#008080>字节</font>"
			 << ",&nbsp;"
			 << "<a href=/yc-cgi-bin/index/Snapshot?"
			 << "word=" << word << "&"
			 << "url=" << url
			 << " target=_blank>"
			 << "[网页快照]</a>"
			 << endl
			 << "<br>";

		if (length > 400 * 1024)
		{ // if more than 400KB
			delete[] pContent;
			continue;
		}

		//LB_c: 以下是从网页数据中提取正文，然后从正文中提取网页摘要，并进行显示。这里不详细解释。
		// skip HEAD
		int bytesRead = 0, newlines = 0;
		while (newlines != 2 && bytesRead != HEADER_BUF_SIZE - 1)
		{
			if (*s == '\n')
				newlines++;
			else
				newlines = 0;
			s++;
			bytesRead++;
		}
		if (bytesRead == HEADER_BUF_SIZE - 1)
			continue;

		// skip header
		bytesRead = 0, newlines = 0;
		while (newlines != 2 && bytesRead != HEADER_BUF_SIZE - 1)
		{
			if (*s == '\n')
				newlines++;
			else
				newlines = 0;
			s++;
			bytesRead++;
		}
		if (bytesRead == HEADER_BUF_SIZE - 1)
			continue;

		CDocument iDocument;
		iDocument.RemoveTags(s);
		iDocument.m_sBodyNoTags = s;
		delete[] pContent;
		string line = iDocument.m_sBodyNoTags;
		CStrFun::ReplaceStr(line, "&nbsp;", " ");
		CStrFun::EmptyStr(line); // set " \t\r\n" to " "

		// abstract
		string reserve;
		if ((unsigned char)line.at(48) < 0x80)
		{
			reserve = line.substr(0, 48);
		}
		else
		{
			reserve = line.substr(0, 48 + 1);
		}
		reserve = "[" + reserve + "]";
		unsigned int resNum = 128;
		if (vecQuery.size() == 1)
			resNum = 256;
		for (unsigned int i = 0; i < vecQuery.size(); i++)
		{
			string::size_type idx = 0, cur_idx;
			idx = line.find(vecQuery[i], idx);
			if (idx == string::npos)
				continue;
			if (idx > resNum)
			{
				cur_idx = idx - resNum;
				while ((unsigned char)line.at(cur_idx) > 0x80 && cur_idx != idx)
				{
					cur_idx++;
				}
				reserve += line.substr(cur_idx + 1, resNum * 2);
			}
			else
			{
				reserve += line.substr(idx, resNum * 2);
			}
			reserve += "...";
			// highlight
			string newKey = "<font color=#e10900>" + vecQuery[i] + "</font>";
			CStrFun::ReplaceStr(reserve, vecQuery[i], newKey);
		}
		line = reserve;
		cout << line << endl
			 << endl;
		//========================================================================================================
	}

	cout << "</ol>";
	cout << "<br><br><hr><br>";
	cout << "&copy 2004 北大网络实验室<br><br>\n";
	cout << "</center></body>\n<html>";

	return true;
}

//关于网页快照功能的实现在Snapshot.cpp中，本系列文章中不展开进行详细解释。但是有一点在这里指出一下，从Snapshot.cpp
//源代码中得知，快照功能处理的cgi程序根据传入的网页url从原始网页数据库中读出网页数据显示出来，而查找网页数据也处理的
//很复杂，先加载url索引文件，再根据传入url的MD5值到文件中找出相应的docid，然后从原始网页数据库中找到该网页的数据再
//进行显示。为何这样处理呢? 在ShowBelow中不是已经得到结果网页的网页数据了吗，可以缓存下来，需要显示网页快照时直接
//取出进行显示不就可以了吗?

bool CDisplayRst::ShowTop()
{
	string strHost = string(getenv("HTTP_HOST"));
	//LB_c: 结果网页的body标签
	cout << "<body bgcolor=#ffffff topmargin=2 marginheight=2>"
		 << "<table class=border=0 width=100% cellspacing=0 cellpadding=0 height=29>" << endl
		 << "<tr>" << endl
		 //LB_c: 左上角的"天网搜索"的logo图片，给该图片加了一个到strHost/yc/TSE/的链接(strHost是网站root目录，即apache配置的网站目录，
		 //前面已经设置为/var/www/html/)。这样，点击logo会打开strHost/yc/TSE/中的index.html，该网页与strHost中的index.html实际上是一样的，
		 //也就是搜索主页，不知道为什么不直接链接到strHost中的index.html?
		 << "<td width=36% rowspan=2 height=1>"
		 << "<a href=http://"
		 << strHost << "/yc/TSE/><img border=0 src=/yc/TSE/tsetitle.JPG width=308 height=65></a></td>" << endl
		 //LB_c: 顶部的"搜索主页"和"使用帮助"链接，前者与上面的logo链接一样，后者链接到一个网络上的帮助手册。
		 << "<td width=64% height=33 ><font size=2><a href=http://"
		 << strHost << "/yc/TSE/>搜索主页</a>| <a href=http://e.pku.edu.cn/gbhelp.htm>使用帮助</a> </font><br></td>" << endl
		 << "</tr>" << endl;

	//LB_c: 这里在网页顶部构建了一个新查询的form，包括搜索框、搜索按钮和隐含的显示页码键值对
	cout << "<tr>" << endl
		 << "<td><p align=\"left\">" << endl
		 << "<form method=\"get\" action=\"/yc-cgi-bin/index/TSESearch\" name=\"tw\">" << endl
		 //LB_c: 搜索输入框
		 << "<input type=\"text\" name=\"word\" size=\"55\">" << endl
		 //LB_c: 新查询按钮，注意该input标签中没有设置name
		 << "<INPUT TYPE=\"submit\" VALUE=\" 新查询 \">&nbsp;" << endl
		 //LB_c: 附加的键值对，键名为"start"，值为1，这里是指示显示搜索结果的第1页。
		 << "<input type=\"hidden\" name=\"start\" value=\"1\">" << endl
		 << "</form>" << endl
		 << "</tr>" << endl
		 << "</table>" << endl;

	//LB_c: 中间蓝色的横条，上面写有"图片"
	cout << "<table border=0 width=100% cellspacing=1 cellpadding=0 height=1>" << endl
		 << "<tr>" << endl
		 << "<td width=68 align=center bgcolor=#000066 valign=middle><font size=2><b><font color=#FFFFFF>图  片</font></b></font></td>"
		 << endl
		 << "</tr>" << endl
		 << "<tr>"
		 << "<td width=100% align=left colspan=3 height=0>"
		 << "</td></tr>" << endl
		 << "</table>" << endl;

	return true;
}
//第6节中提到过如果从搜索结果页面中进行新的搜索，显示结果是正常的，为什么呢? 看一下代码中的新查询的form，form中有
//三个input，而只有两个input有name，所以用户点击"新查询"按钮后，提交的URL中有两个键值对(word和start)，例如查询"北京大学"得到
//http://localhost:8080/yc-cgi-bin/index/TSESearch?word=%B1%B1%BE%A9%B4%F3%D1%A7&start=1，因此HtmlInputs[1]就对应start键值对，
//HtmlInputs[1].Value的值为1，所以m_iStart设置为1，即显示结果集的第1页。
//这里如果在"新查询"按钮的input标签中定义了name，则URL会有三个键值对，第二个键值对对应"新查询"按钮，HtmlInputs[1]就不是start键值对了，
//所以也会出错! 读者朋友们可以测试一下。