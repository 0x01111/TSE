#ifndef _Query_H_040515_
#define _Query_H_040515_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "Document.h"
using namespace std;

const int MaxNameLength = 255;
const int MaxValueLength = 2000;
const int MAXFORMITEM = 40;

typedef struct {
	char Name[MaxNameLength]; 
	char Value[MaxValueLength];
}HtmlInput_Struct;


class CQuery
{
public:
	string m_sQuery;
	string m_sSegQuery;
	unsigned m_iStart;

private:
	int HtmlInputCount;
	HtmlInput_Struct HtmlInputs[MAXFORMITEM];

public:
	CQuery();
	~CQuery();

	void ParseQuery(vector<string> &);

	bool GetInvLists(map<string, string> &) const;
	//bool GetDocidUrl(map<string, string> &) const;
	bool GetDocIdx(vector<DocIdx> &) const;

	int GetInputs();


	// p1: &vecTerm, p2:&mapBuckets, p3: &setRelvantRst
	bool GetRelevantRst(vector<string> &, map<string,string> &, set<string> &) const;

	bool RecordQueryLog() const;

	void SetQuery();
	void SetQuery(string & q);
	void SetStart();

};



#endif // _Query_H_040515_
