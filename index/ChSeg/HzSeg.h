#ifndef _HZSEG_H_040415_
#define _HZSEG_H_040415_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include "Dict.h"

using namespace std;

class CHzSeg
{
public:
	CHzSeg();
	~CHzSeg();

	string SegmentSentenceMM (CDict&, string) const;
	string SegmentHzStrMM (CDict&, string) const;
	string SegmentURL(CDict&, string) const;

	// process a sentence before segmentation
	void Translate(char* SourceStr) const;
};
	
#endif /* _HZSEG_H_040415_ */
