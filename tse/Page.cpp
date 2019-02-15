/*Page handling
 */

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <iterator>
#include "Url.h"
#include "Page.h"
#include "StrFun.h"

CPage::CPage()
{
	m_nStatusCode = 0;
	m_nContentLength = 0;
	m_sLocation = "";
	m_bConnectionState = false;
	m_sContentEncoding = "";
	m_sContentType = "";
	m_sCharset = "";
	m_sTransferEncoding = "";

	m_sContentLinkInfo = "";
        m_sLinkInfo4SE = "";
        m_sLinkInfo4History = "";

        m_sContentNoTags = "";
	m_nRefLink4SENum = 0;
	m_nRefLink4HistoryNum = 0;
        m_eType = PLAIN_TEXT;

	for(int i=0; i< MAX_URL_REFERENCES; i++ ){
		m_RefLink4SE[i].link = NULL;
		m_RefLink4SE[i].anchor_text = NULL;
		m_RefLink4SE[i].strCharset = "";

		if(i < MAX_URL_REFERENCES/2){
			m_RefLink4History[i].link = NULL;
		}
	}

}

CPage::CPage( string strUrl, string strLocation, char* header, char* body, int nLenBody)
{
	m_nStatusCode = 0;
	m_nContentLength = 0;
	m_sLocation = "";
	m_bConnectionState = false;
	m_sContentEncoding = "";
	m_sContentType = "";
	m_sCharset = "";
	m_sTransferEncoding = "";

	m_sContentLinkInfo = "";
    m_sLinkInfo4SE = "";
    m_sLinkInfo4History = "";

    m_sContentNoTags = "";
	m_nRefLink4SENum = 0;
	m_nRefLink4HistoryNum = 0;
    m_eType = PLAIN_TEXT;

	for(int i=0; i< MAX_URL_REFERENCES; i++ ){
		m_RefLink4SE[i].link = NULL;
		m_RefLink4SE[i].anchor_text = NULL;
		m_RefLink4SE[i].strCharset = "";

		if(i < MAX_URL_REFERENCES/2){
			m_RefLink4History[i].link = NULL;
		}
	}

	m_sUrl = strUrl;
	m_sLocation = strLocation;
	m_sHeader = header;
	m_nLenHeader = strlen(header);

	m_sContent.assign(body, nLenBody);
	m_nLenContent = nLenBody;

}

CPage::~CPage()
{
}

void CPage::ParseHeaderInfo(string strHeader)
{
	GetStatusCode(strHeader);
	GetContentLength(strHeader);
	GetLocation(strHeader);
	GetConnectionState(strHeader);
    GetCharset(strHeader);

    GetContentEncoding(strHeader);
    GetContentType(strHeader);
	GetTransferEncoding(strHeader);
}

void CPage::GetStatusCode(string headerBuf)
{
	CStrFun::Str2Lower( headerBuf, headerBuf.length() );

	char *charIndex = strstr(headerBuf.c_str(), "http/");
	if (charIndex == NULL)
	{
		m_nStatusCode = -1;
		return;
	}

	while(*charIndex != ' '){
		charIndex++;
	}
	charIndex++;
	
	int ret = sscanf(charIndex, "%i", &m_nStatusCode);
	if (ret != 1)  m_nStatusCode = -1;
}

void CPage::GetContentLength(string headerBuf)
{
	CStrFun::Str2Lower( headerBuf, headerBuf.length() );

	char *charIndex = strstr(headerBuf.c_str(), "content-length");
	if (charIndex == NULL) return;

	while(*charIndex != ' '){
		charIndex++;
	}
	charIndex++;
	
	int ret = sscanf(charIndex, "%i", &m_nContentLength);
	if (ret != 1)  m_nContentLength = -1;
}

void CPage::GetLocation(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims("\r\n");

	string strBuf =  headerBuf;
	CStrFun::Str2Lower( headerBuf, headerBuf.length() );

	idx = headerBuf.find("location:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("location: ") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			//m_sLocation = headerBuf.substr(pre_idx, idx - pre_idx);
			m_sLocation = strBuf.substr(pre_idx, idx - pre_idx);
		}
	}
}

void CPage::GetCharset(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(" \",;>");

	CStrFun::Str2Lower(headerBuf, headerBuf.size());

	idx = headerBuf.find("charset=");
	if( idx != string::npos) {
		m_sCharset = headerBuf.substr(idx + sizeof("charset=") -1);
	}

	headerBuf = m_sContent;
	headerBuf = headerBuf.substr(0,2024) ;
	CStrFun::Str2Lower( headerBuf, headerBuf.length() );
	idx = headerBuf.find("charset=");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("charset=") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if(idx != string::npos){
			m_sCharset = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
}

void CPage::GetContentEncoding(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims("\r\n");

	CStrFun::Str2Lower( headerBuf, headerBuf.length() );

	idx = headerBuf.find("content-encoding:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("content-encoding: ") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			m_sContentEncoding = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
}

void CPage::GetConnectionState(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n");

	CStrFun::Str2Lower( headerBuf, headerBuf.length() );

	idx = headerBuf.find("connection:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("connection: ") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			string str = headerBuf.substr(pre_idx, idx - pre_idx);
			//cout << "Connection state: " << str << endl;
			//if (str == "close") m_bConnectionState = false;
			if (str == "keep-alive") m_bConnectionState = true;
		}
	}
}

void CPage::GetContentType(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n");

	CStrFun::Str2Lower( headerBuf, headerBuf.size() );

	idx = headerBuf.find("content-type:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("content-type: ") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			m_sContentType = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
}


void CPage::GetTransferEncoding(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n");

	CStrFun::Str2Lower( headerBuf, headerBuf.size() );

	idx = headerBuf.find("transfer-encoding:");
	if ( idx != string::npos)
	{
		pre_idx = idx + sizeof("transfer-encoding: ") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if(idx != string::npos)
		{
			m_sTransferEncoding = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
}

/*
 * Filter spam links
 * If it is, return ture; otherwise false
 */
bool CPage::IsFilterLink(string plink)
{
	if( plink.empty() ) return true;
	if( plink.size() > URL_LEN ) return true;

	string link = plink, tmp;
	string::size_type idx = 0;

	
	CStrFun::Str2Lower( link, link.length() );

	// find two times following symbols, return false
	tmp = link;
	idx = tmp.find("?");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("?");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("-");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("+");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("&");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("&");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("//");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("//");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("http");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("http");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("misc");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("misc");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("ipb");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("ipb");
		if( idx != string::npos ) return true;
	}

	const char *filter_str[]={
	"cgi-bin",	"htbin",	"linder",	"srs5",		"uin-cgi",  // robots.txt of http://www.expasy.org/
	"uhtbin",	"snapshot",	"=+",		"=-",		"script",
	"gate",		"search",	"clickfile",	"data/scop",	"names",
	"staff/",	"enter",	"user",		"mail",	"pst?",
	"find?",	"ccc?",		"fwd?",		"tcon?",	"&amp",
	"counter?",	"forum",	"cgisirsi",	"{",		"}",
	"proxy",	"login",	"00.pl?",	"sciserv.pl",	"sign.asp",
	"<",		">",		"review.asp?",	"result.asp?",	"keyword",
	"\"",		"'",		"php?s=",	"error",	"showdate",
	"niceprot.pl?",	"volue.asp?id",	".css",		".asp?month",	"prot.pl?",
	"msg.asp",	"register.asp", "database",	"reg.asp",	"qry?u",
	"p?msg",	"tj_all.asp?page", ".plot.",	"comment.php",	"nicezyme.pl?",
	"entr",		"compute-map?", "view-pdb?",	"list.cgi?",	"lists.cgi?",
	"details.pl?",	"aligner?",	"raw.pl?",	"interface.pl?","memcp.php?",
	"member.php?",	"post.php?",	"thread.php",	"bbs/",		"/bbs"
	};
	int filter_str_num = 75;

	
	for(int i=0; i<filter_str_num; i++){
		if( link.find(filter_str[i]) != string::npos)
		return true;
	}	

	return false;
}

/////////////////////////////
// just for ImgSE
// e.g: http://www.people.com.cn/GB/tupian/index.html
// 	http://news.xinhuanet.com/photo/
// 	http://photo.tom.com/
/////////////////////////////
// comment previous one and open this one

/*
bool CPage::IsFilterLink(string plink)
{
	if( plink.empty() ) return true;
	if( plink.size() > URL_LEN ) return true;

	return false;

	string link = plink, tmp;
	string::size_type idx = 0;

	
	CStrFun::Str2Lower( link, link.length() );

	const char *filter_str[]={
		"tupian", "photo", "ttjstk"
		};
	int filter_str_num = 3;

	CStrFun::Str2Lower( link, link.length() );

	for(int i=0; i<filter_str_num; i++){
		if( link.find(filter_str[i]) != string::npos)
		return false;
	}	

	return true;
}
*/


/*****************************************************************
** Function name: ParseHyperLinks
** Input argv:
**      --
** Output argv:
**      --
** Return:
        true: success
        false: fail
** Function Description:  Parse hyperlinks from the web page
** Version: 1.0
** Be careful:
*****************************************************************/
bool CPage::ParseHyperLinks()
{
	if( GetContentLinkInfo() == false ) return false;

	if( m_sContentLinkInfo.empty() ) return false;

	bool bFind4SE = false;
	bool bFind4History = false;
	if( GetLinkInfo4SE() ){
		if( FindRefLink4SE() ) bFind4SE = true;
	} 

	if( GetLinkInfo4History() ){
		if( FindRefLink4History() ) bFind4History = true;
	}

	if( !bFind4SE && !bFind4History ){
		 return false;
	}

	//return   GetHref(m_sContentLinkInfo.c_str(), "href", m_listLink4SE);

	return true;
}


/*****************************************************************
** Function name: GetContentLinkInfo
** Input argv:
**      --
** Output argv:
**      --
** Return:
        true: success
        false: fail
** Function Description:  Parse hyperlinks from the web page
** Version: 1.0
** Be careful:
*****************************************************************/
bool CPage::GetContentLinkInfo()
{
	if( m_sContent.empty() ) return false;
	
	m_sContentLinkInfo = m_sContent;

	string& s = m_sContentLinkInfo;

	// transform all separation into one space character
	//CStrFun::ReplaceStr(s, "\t", " ");
	//CStrFun::ReplaceStr(s, "\r", " ");
	//CStrFun::ReplaceStr(s, "\n", " ");
	const string delims(" \t\r\n");
	string::size_type idx=0, pre_idx;

	while( (idx = s.find_first_of(delims, idx)) != string::npos ){
		pre_idx = idx;
		s.replace(idx,1,1,' ');
		idx++;
		while( (idx = s.find_first_of(delims, idx)) != string::npos ){
			if( idx-pre_idx == 1 ){
				s.erase(idx, 1);
			} else {
				break;
			}
		}

		idx--;
	}

	// transform all "<br>" into one space character
	CStrFun::ReplaceStr(s, "<br>", " ");

	if( s.size() < 20 ) return false;

	// Keep only <img ...>, <area ...>,<script ...> and <a href ...> tags.
	string::size_type idxHref=0,idxArea=0,idxImg=0;
	string dest;

	do{
		if( s.empty() ) break;

		idxHref = CStrFun::FindCase(s, "href");
		idxArea = CStrFun::FindCase(s, "<area");
		idxImg = CStrFun::FindCase(s, "<img");

		pre_idx = idxHref > idxArea? idxArea: idxHref;
		pre_idx = idxImg > pre_idx? pre_idx: idxImg;
		if( pre_idx == string::npos) break;

		s = s.substr(pre_idx);
		idx = s.find_first_of('<',1);
		if( idx != string::npos ){
			dest = dest + s.substr(0,idx);
		}else{
			break;
		}

		s = s.substr(idx);
		idxHref=0; idxArea=0; idxImg=0;
	}while(1);

	s = dest;

	
	/* erase all '\' character
	 * too avoid the following situations:
	 *      document.write("<A href=\"/~webg/refpaper/index.html\">t2</A>");
	*/
	CStrFun::EraseStr(s, "\\");

	if( s.size() < 20 ) return false;

	return true;
}

/*****************************************************************
** Function name: GetLinkInfo4SE()
** Input argv:
**      --  
** Output argv:
**      --
** Return:
       true: success
       false: fail
** Function Description:  Get links for SE
** Version: 1.0
** Be careful:
*****************************************************************/
bool CPage::GetLinkInfo4SE()
{
	if( m_sContentLinkInfo.empty() ) return false;

	m_sLinkInfo4SE = m_sContentLinkInfo;
	string& s = m_sLinkInfo4SE;

 	// Keep only <area ...>,and <a href ...> tags.
	string::size_type idxHref=0,idxArea=0,
		idx,pre_idx;
	string dest;

	do{
		if( s.empty() ) break;

		//idxHref = CStrFun::FindCase(s, "<a href");
		idxHref = CStrFun::FindCase(s, "href");
		idxArea = CStrFun::FindCase(s, "<area ");

		pre_idx = idxHref > idxArea? idxArea: idxHref;
		//pre_idx = idxHref;
		if( pre_idx == string::npos) break;

		s = s.substr(pre_idx);
		idx = s.find_first_of('<',1);

		if( !(s.length() < 4) ){
			idxHref = CStrFun::FindCaseFrom(s, "href", 4);
			idx = idx > idxHref ? idxHref: idx;
		}

		if( idx != string::npos ){
			dest = dest + s.substr(0,idx);
		}else if (idx == string::npos && pre_idx != string::npos){
			dest = dest + s;
			break;
		}else{
			break;
		}

		s = s.substr(idx);
		idxHref=0; idxArea=0;
	}while(1);
		
	s = dest;
	if( s.length() < 20 ) return false;


	// erase all '"' , '\'', "&nbsp;".
	CStrFun::EraseStr(s, "\"");
	CStrFun::EraseStr(s, "'");
	CStrFun::EraseStr(s, "&nbsp");

 	// Keep URLs and anchor text.

	idxHref=0;
	const string delims( " #>");
	dest.clear();

	do{
		if( s.empty() ) break;
		idxHref = CStrFun::FindCase(s, "href");

		if( idxHref == string::npos) break;
		pre_idx = idxHref;

		//####
		idx = s.find('=', idxHref);
		if( idx == string::npos ) break;
		s = s.substr(idx+1);

		while( s.length() > 0 && s[0] == ' ' ){
			s.erase(0,1);
		}
		if( s.length() == 0 ) break;

		idx = s.find_first_of(delims,1);
		//cout << endl << s.substr(0, idx) << endl;
		if( idx == string::npos ) break;

		dest += '"' + s.substr(0, idx);

		//cout << endl << dest << endl;
			
		idx = s.find('>');
		if( idx == string::npos ) break;
		dest += '>';
		s = s.substr(idx +1);
			
		idx = s.find('<');

		if( !s.empty() ){
			idxHref = CStrFun::FindCase(s, "href");
			idx = idx > idxHref ? idxHref: idx;
		}	

		if( idx == string::npos ){
			dest += s;
			break;
		}

/*
		if( idx == idxHref ){
			dest += '"' + s.substr(0,idx);
		}else{
*/
			dest += s.substr(0,idx);
		//}
		//####

		idxHref=0;
	}while(1);
		
	// look for empty filenames.
	idx = 0;
	while( (idx = dest.find("\"\"",idx)) != string::npos ){
		dest.erase(idx, 1);
	}

	s = dest;

	return( s.length() < 20 ? false: true );

}
					
/*****************************************************************
** Function name: GetLinkInfo4History()
** Input argv:
**      --  
** Output argv:
**      --
** Return:
       true: success
       false: fail
** Function Description:  Get links for history archiving
** Version: 1.0
** Be careful:
*****************************************************************/
bool CPage::GetLinkInfo4History()
{
	if( m_sContentLinkInfo.empty() ) return false;

	m_sLinkInfo4History = m_sContentLinkInfo;
	string& s = this->m_sLinkInfo4History;

 	// Keep only <img ...> tags.
	string::size_type idxImg=0,
		idx,pre_idx;
	string dest;

	do{
		if( s.empty() ) break;
		idxImg = CStrFun::FindCase(s, "<img");

		pre_idx = idxImg;
		if( pre_idx == string::npos) break;

		s = s.substr(pre_idx);
		idx = s.find_first_of('<',1);

		if( idx != string::npos ){
			dest = dest + s.substr(0,idx);
		}else if (idx == string::npos && pre_idx != string::npos){
			dest = dest + s;
			break;
		}else{
			break;
		}

		s = s.substr(idx);
		idxImg=0;
	}while(1);
		
	s = dest;
	if( s.length() < 20 ) return false;

	// erase all '"'. '\'',"&nbsp;".
	CStrFun::EraseStr(s , "\"");
	CStrFun::EraseStr(s , "'");
	CStrFun::EraseStr(s , "&nbsp");

 	// Keep URLs and anchor text.

	idxImg=0;
	string::size_type idxSrc = 0;
	const string delims( " #>");
	dest.clear();

	do{
		if( s.empty() ) break;
		idxImg = CStrFun::FindCase(s, "img");

		if( idxImg == string::npos) break;
		pre_idx = idxImg;

		s = s.substr(idxImg+3);		// skip "img"

		//####
		idx = s.find('>', idxImg);
		if( idxImg == string::npos) break;
		if( s.empty() ) break;
		idxSrc = CStrFun::FindCase(s, "src");
		if( idxSrc > idxImg ) continue;
		s = s.substr(idxSrc);

		idx = s.find('=', idxImg);
		if( idx == string::npos ) break;
		s = s.substr(idx+1);

		while( s.length() > 0 && s[0] == ' ' ){
			s.erase(0,1);
		}
		if( s.length() == 0 ) break;

		idx = s.find_first_of(delims,1);
		if( idx == string::npos ) break;

		if( s.at(0) == '"'){
			dest += s.substr(0, idx);
		}else{
			dest += '"' + s.substr(0, idx);
		}
			
		idx = s.find('>');
		if( idx == string::npos ) break;
		dest += '>';
		s = s.substr(idx +1);
			
		idx = s.find('<');
		if( idx == string::npos ){
			dest += s;
			break;
		}
		dest += s.substr(0,idx);
		//####

		idxImg=0;
	}while(1);
		

	// look for empty filenames.
	idx = 0;
	while( (idx = dest.find("\"\"",idx)) != string::npos ){
		dest.erase(idx, 1);
	}

	s = dest;

	return( s.length() < 20 ? false: true );

}


bool CPage::NormalizeUrl(string& strUrl)
{
	string::size_type idx;

	if( CStrFun::FindCase(strUrl, "http://") == string::npos ) return false;

	// convert "http://e.pku.cn" to "http://e.pku.cn/"
	idx = strUrl.rfind('/');
	if( idx < 8 ) {
		strUrl = strUrl + "/";
		return true;
	}

	while( (idx=strUrl.find("/./")) != string::npos ){
		if( idx != string::npos ) strUrl.erase(idx,2);
	}

	while( (idx = strUrl.find("/../")) != string::npos ){
		string strPre,strSuf;

		strPre = strUrl.substr(0, idx);

		if( strUrl.length() > idx+4 )
			strSuf = strUrl.substr(idx+4);

		idx = strPre.rfind("/");
		if( idx != string::npos)
			strPre = strPre.substr(0,idx+1);
		if( strPre.length() < 10 ) return false;

		strUrl = strPre + strSuf;
	}

	if( CStrFun::FindCase(strUrl, "http://") != 0 ) return false;

	return true;
}

bool CPage::FindRefLink4SE()
{
	if( m_sLinkInfo4SE.empty() ) return false;

	char *buffer = (char*)m_sLinkInfo4SE.c_str();
	int urlnum=0,len;
	char *ptr ;

	static char buf[URL_REFERENCE_LEN];

	memset(buf, 0, URL_REFERENCE_LEN);
	len = strlen(buffer);
	if( len < 8 ) return false;

	len = len < URL_REFERENCE_LEN -1 ? len : URL_REFERENCE_LEN - 1;
	strncpy( buf, buffer, len);

/*first
 *------>
 */
	ptr = buf;
	while( ptr - buf < len  && *ptr ){
		while( *ptr == '"' && *ptr) ptr++;
		if ( !*ptr ) break;
		this->m_RefLink4SE[ urlnum].link = ptr;
		while( *ptr && *ptr != '>'){
			if(*ptr == ' ') *ptr = '\0';
			ptr++;
		}

		if ( !*ptr ){
			urlnum++;
			break;
		}
		if ( *ptr == '>' ){
			*ptr++='\0';
			if( !*ptr ){
				urlnum++;
				break;
			}
			if( *ptr == '"' ){
				this->m_RefLink4SE[urlnum].anchor_text = NULL;
			}else{
				this->m_RefLink4SE[urlnum].anchor_text = ptr;
				while( *ptr && *ptr != '"') ptr++;
				if (!*ptr){
					urlnum++;
					break;
				}
				if ( *ptr == '"') *ptr='\0';
			}

		}
		
		//cout << endl << this->m_RefLink4SE[ urlnum].link << '\t';
		//cout << this->m_RefLink4SE[ urlnum].anchor_text << endl;

		ptr++;
		urlnum++;
		if ( urlnum == MAX_URL_REFERENCES) break;
	}
	//cout << endl << this->m_RefLink4SE[ urlnum].link << endl;
	//cout << this->m_RefLink4SE[ urlnum].anchor_text << endl;

	this->m_nRefLink4SENum = urlnum;

/*second
 *------>
 */
	//typedef map<string,string,less<string> >::value_type valType;
	typedef map<string,string>::value_type valType;

	m_mapLink4SE.clear();

	//string strRootUrl= m_sUrl;
	CUrl iUrl;
	if( iUrl.ParseUrlEx(m_sUrl) == false ){
		cout << "ParseUrlEx error in FindRefLink4SE(): " << m_sUrl << endl;
		return false;
	}
	
	for(int i=0; i<m_nRefLink4SENum; i++){

		string str;
		string::size_type idx;
		const string delims(" #");

		str = m_RefLink4SE[i].link;
		idx = str.find_first_of(delims, 0 );
		if( idx != string::npos ){
			str = str.substr(0, idx);
		}
		if( str.size() == 0 || str.size() > URL_LEN - 1 
			|| str.size() < 4 ) continue;


		string::size_type idx1;
		idx1 = CStrFun::FindCase(str, "http");
		if( idx1 != 0  ){
			char c1 = m_sUrl.at(m_sUrl.length()-1);
			char c2 = str.at(0);

			if( c2=='/' ){
				if( iUrl.m_nPort != 80 ){
					cout << iUrl.m_sHost << endl;
					cout << str << endl;
					//str = "http://" + iUrl.m_sHost + ":" + (const char*)(iUrl.m_nPort) + str;
					str = "http://" + iUrl.m_sHost + ":" + CStrFun::itos(iUrl.m_nPort) + str;
				} else {
					str = "http://" + iUrl.m_sHost + str;
				}
			} else if( c1!='/' && c2!='/'){
				string::size_type idx;

				idx = m_sUrl.rfind('/');
				if( idx != string::npos ){
					if( idx > 6 ){ // > strlen("http://..")
						str = m_sUrl.substr(0, idx+1) + str;
					} else {
						str = m_sUrl + "/" + str;
					}

				} else {

					continue;
				}

			} else {
				if( c1=='/' ){
					str = m_sUrl + str;
				} else {
					str = m_sUrl + "/" + str;
				}
			}
		}

		if( NormalizeUrl(str) == false ) continue;

		if( IsFilterLink(str) ) continue;

		//debug
		//cout << "reflink: " << reflink << endl;

		if( str == m_sUrl ){
			continue;
		}else{
			if( m_RefLink4SE[i].anchor_text ){
				if( m_mapLink4SE.find(str) == m_mapLink4SE.end() ){
					m_mapLink4SE.insert( valType( str, m_RefLink4SE[i].anchor_text));
				}
			}else{
				if( m_mapLink4SE.find(str) == m_mapLink4SE.end() ){
					m_mapLink4SE.insert( valType( str, "\0") );
					cout << ".";
				}
			}
		}
			

	}

	m_nRefLink4SENum = m_mapLink4SE.size();

	//cout << endl;

	return true;
}

bool CPage::FindRefLink4History()
{
	if( m_sLinkInfo4History.empty() ) return false;

	char *buffer = (char*)m_sLinkInfo4History.c_str();
	int urlnum=0,len;
	char *ptr ;

	static char buf[URL_REFERENCE_LEN/2];

	memset(buf, 0, URL_REFERENCE_LEN/2);
	len = strlen(buffer);
	if( len < 8 ) return false;

	len = len < URL_REFERENCE_LEN/2 - 1? len : URL_REFERENCE_LEN/2 -1;
	strncpy( buf, buffer, len);

/*first
 *------>
 */
	ptr = buf;
	while( ptr - buf < len  && *ptr ){
		while( *ptr == '"' && *ptr) ptr++;
		if ( !*ptr ) break;
		this->m_RefLink4History[ urlnum].link = ptr;

		while( *ptr && *ptr != '>'){
			if( *ptr == ' ') *ptr='\0';
			ptr++;
		}

		if( !*ptr){
			urlnum++;
			break;
		}
		if( *ptr == '>' ){
			*ptr++ = 0;
			if( !*ptr ){
				urlnum++;
				break;
			}
			if( *ptr == '"' ){
			
			}else{
				while( *ptr && *ptr != '"') ptr++;
				if( !*ptr ){
					urlnum++;
					break;
				}
				if ( *ptr == '"' ) *ptr++='\0';
			}
		}
		
		ptr++;
		urlnum++;
		if ( urlnum == MAX_URL_REFERENCES/2) break;
	}


	this->m_nRefLink4HistoryNum = urlnum;

/*second
 *------>
 */
	m_vecLink4History.clear();
	//string strRootUrl= m_sUrl;
        CUrl iUrl;
        if( iUrl.ParseUrlEx(m_sUrl) == false ){
		cout << "ParseUrlEx error in FindRefLink4History(): " << m_sUrl << endl;
		return false;
	}

	for(int i=0; i<m_nRefLink4HistoryNum; i++){
		string str;
		//string::size_type idx;

		str = m_RefLink4History[i].link;
		if( str.size()==0 || str.size() > URL_LEN - 1 
			|| str.size() < 4 ) continue;

/*
		char *pdest1, *pdest2;
		pdest1 = strstr( str.c_str(), "http" );
		pdest2 = strstr( str.c_str(), "HTTP" );
		if( pdest1==NULL && pdest2==NULL ){
*/

		string::size_type idx1;
		idx1 = CStrFun::FindCase(str, "http");
		if( idx1 != 0 ){
			char c1 = m_sUrl.at(m_sUrl.length()-1);
			char c2 = str.at(0);

			if( c2=='/' ){
				if( iUrl.m_nPort != 80 ){
					str = "http://" + iUrl.m_sHost + ":" + CStrFun::itos(iUrl.m_nPort) + str;
				} else {
					str = "http://" + iUrl.m_sHost + str;
				}
			} else if( c1!='/' && c2!='/'){
				string::size_type idx;

				idx = m_sUrl.rfind('/');
				if( idx != string::npos ){
					if( idx > 6 ){ // > strlen("http://..")
						str = m_sUrl.substr(0, idx+1) + str;
					} else {
						str = m_sUrl + "/" + str;
					}

				} else {

					continue;
				}

			} else {
				if( c1=='/' ){
					str = m_sUrl + str;
				} else {
					str = m_sUrl + "/" + str;
				}
			}
		}

		// due to bad link parser
/*

		idx = reflink.find(' ');
		if(idx != string::npos){
			reflink = reflink.substr(0,idx);
		}
		idx = reflink.find('"');
		if(idx != string::npos){
			reflink = reflink.substr(0,idx);
		}
*/
		//#############

		if( NormalizeUrl(str) == false ) continue;


		if( IsFilterLink(str) ) continue;


		if( str == m_sUrl ){
			continue;
		}else{
			vector<string>::iterator it;
			it = find(m_vecLink4History.begin(), m_vecLink4History.end(),str);
			if( it == m_vecLink4History.end() ){

				m_vecLink4History.push_back( str);
				cout << ".";
			}
		}
			

	}
	m_nRefLink4HistoryNum = m_vecLink4History.size();
	//cout << endl;

	return true;
}
