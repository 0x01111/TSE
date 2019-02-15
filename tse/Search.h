#ifndef _SEARCH_H_031105_
#define _SEARCH_H_031105_

class CSearch
{
public:
	CSearch();
	virtual ~CSearch();

	void DoSearch();

private:
	int FindKey(const char* key);
	int FindUrl(const char* url, char **content);
};

#endif /* _SEARCH_H_031105_ */
