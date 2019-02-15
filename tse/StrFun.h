#ifndef _STR_FUN_H_030802
#define _STR_FUN_H_030802

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

class CStrFun
{
public:
	CStrFun();
	virtual ~CStrFun();

	static void Str2Lower(string &sSource, int nLen);
	static string itos(long long i){
		stringstream s;
		s << i;
		return s.str();
	}

	/* Locate a substring ignoring case
	 * The  function  returns a value equal or lager than zero,
         * or -1 if the substring is not found.
	 */

	static string::size_type FindCase(string haystack, string needle);
	static string::size_type FindCaseFrom(string haystack, string needle, int From);

	static void ReplaceStr(string &str, string srstr, string dsstr);
	static void EraseStr(string &str, string substr);
};

#endif	// end _STR_FUN_H_030802

/*
ostringstream oss<<Dea<<'.'<<pre();

oss.str();
*/
