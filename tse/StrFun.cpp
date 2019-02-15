#include "StrFun.h"

CStrFun::CStrFun()
{

}

CStrFun::~CStrFun()
{

}

void CStrFun::Str2Lower(string& str, int nLen)
{
        char distance='A'-'a' ;

        for( int i=0 ; i<nLen; i++ )
                if  ( str[i]>='A' && str[i]<='Z' )
                        str[i] -= distance ;
}

bool _nocase_compare(char c1, char c2)
{
        return toupper(c1) == toupper(c2);
}

string::size_type CStrFun::FindCase(string haystack, string needle)
{
	if (haystack.empty()) return string::npos;
	if (needle.empty()) return string::npos;

	string::iterator pos;
	pos = search(haystack.begin(), haystack.end(), 
		needle.begin(),needle.end(), _nocase_compare);

	if( pos == haystack.end()){
		return string::npos;
	}else{
		return (pos - haystack.begin());
	}
}

string::size_type CStrFun::FindCaseFrom(string haystack, string needle, int nFrom)
{
	//assert( haystack.empty() == false );
	//assert( needle.empty() == false );
	if (haystack.empty()) return string::npos;
	if (needle.empty()) return string::npos;

	string::iterator pos;
	pos = search(haystack.begin()+nFrom, haystack.end(), 
		needle.begin(),needle.end(), _nocase_compare);

	if( pos == haystack.end()){
		return string::npos;
	}else{
		return (pos - haystack.begin());
	}
}

void CStrFun::EraseStr(string &str , string substr)
{
	if( str.size() == 0 || substr.size() == 0 )
		return;

	string::size_type idx = 0;
	string::size_type sub_length = substr.length();
	idx = str.find(substr,idx);
	while( idx != string::npos ){
		str.erase(idx,sub_length);
		idx = str.find(substr,idx);
	}
}

void CStrFun::ReplaceStr(string&str,string srstr,string dsstr)
{
	if( str.size() ==0 || srstr.size() == 0 )
		return;

	string::size_type idx = 0;
	string::size_type sub_length = srstr.length();
	idx = str.find(srstr,idx);
	while( idx != string::npos ){
		str.replace(idx,sub_length,dsstr);

		if( idx+dsstr.size() > str.size() ) break;

		idx = str.find(srstr,idx+dsstr.size());
	}
}

