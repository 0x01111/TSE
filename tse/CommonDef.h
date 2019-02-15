#ifndef _COMMON_DEF_H_030717_
#define _COMMON_DEF_H_0300717_

const int PORT_NUMBER = 80;
#define HTTP_VERSION "HTTP/1.0"
#define DEFAULT_USER_AGENT	 "Tse"
#define VERSION	 "1.0"
const int DEFAULT_TIMEOUT = 30;	/* Seconds to wait before giving up when no data is arriving */
const int REQUEST_BUF_SIZE = 1024;
const int HEADER_BUF_SIZE = 1024;
const int DEFAULT_PAGE_BUF_SIZE = 1024 * 200;      /* 200K should hold most things */
const int MAX_PAGE_BUF_SIZE = 5 * 1024 * 1024;      /* 5MB is up limit */

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////////
// collection
extern map<string,string> mapCacheHostLookup;

extern map<unsigned long,unsigned long> mapIpBlock;
typedef map<unsigned long,unsigned long>::value_type valTypeIpBlock;

extern set<string> setVisitedUrlMd5;
////////////////////////////

#endif /* _COMMON_DEF_H_0300717_ */
