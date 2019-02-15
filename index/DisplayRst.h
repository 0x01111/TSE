#ifndef _DISPLAY_RST_H
#define _DISPLAY_RST_H

#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>

#include "Document.h"

using namespace std;

class CDisplayRst
{
public:
	CDisplayRst();
	~CDisplayRst();
		
	bool ShowTop();
	bool ShowMiddle(string,float,unsigned,unsigned);
	bool ShowBelow(vector<string>&, set<string> &, vector<DocIdx> &, unsigned);
private:
		

};

#endif

