// Dict handling

#include "Dict.h"

CDict::CDict()
{
	//LB_c: 读取字典文件，初始化mapDict 
	OpenDict();
}

CDict::~CDict()
{
	mapDict.clear();
}

//LB_c: 打开字典文件DICTFILENAME，读取所有词存入成员变量mapDict.
void CDict::OpenDict()
{
	FILE *fpDict;
	if ((fpDict = fopen(DICTFILENAME.c_str(), "r")) == NULL) {
		cout << "Can not open the Dictionary file!";
		exit(1);
	}
	int id, freq;
	char word[16];
    int limit = 100;
	while (fscanf(fpDict, "%d %s %d", &id, word, &freq) != EOF) {
    //    fscanf(fpDict, "%d %s %d", &id, word, &freq);
		mapDict.insert(map<string,int>::value_type (word, freq));
	}
	fclose(fpDict);
  
}

bool CDict::IsWord(string& str) const
{
	if (mapDict.find(str) != mapDict.end())
		return true;
	return false;
}
