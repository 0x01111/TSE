#ifndef _TSE_H_030719_
#define _TSE_H_030719_

#ifndef		_STRING_
#define		_STRING_
#include	<string>
#endif

using namespace std;

const string DATA_FILE_NAME("WebData.db");
const string INDEX_FILE_NAME("WebData.idx");

const string DATA_TIANWANG_FILE("Tianwang.raw");
const string DATA_LINK4SE_FILE("Link4SE.raw");

const string VISITED_FILE("visited.url");
const string UNVISITED_FILE("tse_unvisited.url");
const string UNREACH_HOST_FILE("tse_unreachHost.list");
const string LINK4SE_FILE("link4SE.url");
const string LINK4History_FILE("link4History.url");
const string URL_MD5_FILE("tse_md5.visitedurl");
const string PAGE_MD5_FILE("tse_md5.visitedpage");

const string IP_BLOCK_FILE("tse_ipblock");

const unsigned int NUM_WORKERS		= 10;
//const unsigned int NUM_WORKERS		= 1;
const unsigned int NUM_WORKERS_ON_A_SITE = 4;

/*============== Include file ==================*/

#ifndef         _IOSTREAM_
#define         _IOSTREAM_
#include        <iostream>
#endif

#ifndef         _FSTREAM_
#define         _FSTREAM_
#include        <fstream>
#endif

#ifndef         _STDIO_H_
#define         _STDIO_H_
#include        <cstdio>
#endif

#ifndef         _STDLIB_H_
#define         _STDLIB_H_
#include        <stdlib.h>
#endif

#ifndef         _CSTDLIB_
#define         _CSTDLIB_
#include        <cstdlib>
#endif

#ifndef         _CSTRING_
#define         _CSTRING_
#include        <cstring>
#endif

#ifndef         _DIRENT_H_
#define         _DIRENT_H_
#include        <dirent.h>
#endif

#ifndef         _UNISTD_H_
#define         _UNISTD_H_
#include        <unistd.h>
#endif

#ifndef         _S_DIR_H_
#define         _S_DIR_H_
#include        <sys/dir.h>
#endif

#ifndef         _S_TYPES_H_
#define         _S_TYPES_H_
#include        <sys/types.h>
#endif

#ifndef         _S_STAT_H_
#define         _S_STAT_H_
#include        <sys/stat.h>
#endif

#ifndef         _FTW_H_
#define         _FTW_H_
#include        <ftw.h>
#endif

#ifndef         _ERROR_H_
#define         _ERROR_H_
#include        <error.h>
#endif

#ifndef		_STREAMBUF_
#define		_STREAMBUF_
#include 	<streambuf>
#endif

// #ifndef		_OPS_MD5_H_
// #define		_OPS_MD5_H_	
// #include	 <openssl/md5.h>
// #endif

#ifndef		_IOMANIP_
#define		_IOMANIP_	
#include	 <iomanip>
#endif

#ifndef		_CTIME_
#define		_CTIME_
#include 	<ctime>
#endif

#ifndef		_ALGORITHM_
#define		_ALGORITHM_
#include 	<algorithm>
#endif

#ifndef		_CCTYPE_
#define		_CCTYPE_
#include 	<cctype>
#endif


#ifndef		_VECTOR_
#define		_VECTOR_
#include 	<vector>
#endif

#ifndef		_ITERATOR_
#define		_ITERATOR_
#include	<iterator>
#endif

#ifndef		_LIST_
#define		_LIST_
#include 	<list>
#endif

#ifndef		_DEQUE_
#define		_DEQUE_
#include 	<deque>
#endif

#ifndef		_MAP_
#define		_MAP_
#include 	<map>
#endif

#ifndef		_SET_
#define		_SET_
#include 	<set>
#endif

#ifndef		_CASSERT_
#define		_CASSERT_
#include	<cassert>
#endif

#ifndef		_SIGNAL_H_
#define		_SIGNAL_H_
#include	<signal.h>
#endif

#ifndef		_SOCKET_H_	
#define		_SOCKET_H_
#include	<sys/socket.h>
#endif

#ifndef		_IN_H_
#define		_IN_H_
#include	<netinet/in.h>
#endif

#ifndef		_INET_H_
#define		_INET_H_
#include	<arpa/inet.h>
#endif
/*==========================================================================*/

/*
#ifdef		_DMALLOC_
#include	<dmalloc.h>
#endif		 
*/

/*==========================================================================*/

#endif /* _TSE_H_030719_ */

