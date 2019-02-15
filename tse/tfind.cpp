#include <iostream>
#include <string>

using namespace std;
const unsigned int URL_LEN=256;

bool IsFilterLink(string plink);

int main()
{

	string str="http://cen.ccer.edu.cn/cn/daohang/o?href=//cn/ReadNews.asp?NewsID=6122 class=red>?D??D??¡éhref=/cn/ReadNews.asp";
	string str1="http://www.cenet.org.cn/ttp//cn/tougao/ougao/list.asp";
	string str2="http://162.105.80.57/scripts/SnapShot.exe?SortType=1&Site=//202.120.8.76/bin0/bin/ftp://202.120.8.76/bin0/bin/awk/";

	if( IsFilterLink(str2) ){
		cout << "find" << endl;
	}

	return 0;
}
	
/*
 * Filter spam links
 * If it is, return ture; otherwise false
 */
bool IsFilterLink(string plink)
{
	if( plink.empty() ) return false;
	if( plink.size() > URL_LEN ) return false;

	string link = plink, tmp;
	string::size_type idx = 0;

	
	//CStrFun::Str2Lower( link, link.length() );

	// find two times following symbols, return false
	tmp = link;
	idx = tmp.find(":");
	if( idx != string::npos ) tmp = tmp.substr(idx+1);
	idx = tmp.find(":");
	if( idx != string::npos ) return false;

	tmp = link;
	idx = tmp.find("?");
	if( idx != string::npos ) tmp = tmp.substr(idx+1);
	idx = tmp.find("?");
	if( idx != string::npos ) return false;

	tmp = link;
	idx = tmp.find("&");
	if( idx != string::npos ) tmp = tmp.substr(idx+1);
	idx = tmp.find("&");
	if( idx != string::npos ) return false;

	tmp = link;
	idx = tmp.find("//");
	if( idx != string::npos ) tmp = tmp.substr(idx+1);
	idx = tmp.find("//");
	if( idx != string::npos ) return false;

	tmp = link;
	idx = tmp.find("http");
	if( idx != string::npos ) tmp = tmp.substr(idx+1);
	idx = tmp.find("http");
	if( idx != string::npos ) return false;


	const char *filter_str[]={
		"gate", "search", "clickfile?dir1", "data/scop", "uhtbin",
		"staff/staff", "enter", "userid", "pstmail?", "pst?",
		"find?", "ccc?", "fwd?", "tcon?", "&amp",
		"Counter?", "forum", "cgisirsi", "{", "}",
		"proxy", "login", "mailto:", "javascript:", "direction=-",
		"direction=+", "cgi-bin/sciserv.pl","sign.asp","<",">",
		"snapshot","cgi-bin/bbs","review.asp?","Result.asp?","keyword",
		"\"","'","SnapShot.exe"
		};
	int filter_str_num = 38;

	
	for(int i=0; i<filter_str_num; i++){
		if( link.find(filter_str[i]) != string::npos)
		return true;
	}	

	return false;
}

