#ifndef _DICT_H_040401_
#define _DICT_H_040401_

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

using namespace std;

const string DICTFILENAME("words.dict");


/*
typedef struct{
	int id;
	string word;
	int freq;
} DictEntry;
*/

class CDict
{
    
public:
	CDict();
	~CDict();
	  
	bool GetFreq(string&) const {return false;};
	bool IsWord(string&) const;
	void AddFreq(string&) {};
	  
private:
	map<string, int> mapDict;
	void OpenDict();	  
};

#endif /* _DICT_H_040401_ */
