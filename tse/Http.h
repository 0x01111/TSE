#ifndef _HTTP_H_031105_
#define _HTTP_H_031105_

#include <map>

using namespace std;

class CHttp
{
private:
	string m_strUrl;	// url
	int *m_sock;		// socket

public:
	CHttp();
	virtual ~CHttp();

	int Fetch(string strUrl, char **fileBuf, char **fileHead, char **location, int* sock);

private:
	int read_header(int sock, char *headerPtr);
	int CreateSocket(const char *host, int port);

	int nonb_connect(int, struct sockaddr*, int);
	int checkBufSize(char **buf, int *bufsize, int more);

};

extern pthread_mutex_t mutexMemory;

#endif /* _HTTP_H_031105_ */
