#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <iostream>
#include "Http.h"

#include "Tse.h"
#include "CommonDef.h"
#include "Url.h"
#include "Page.h"
#include "StrFun.h"

char *userAgent = NULL;
int timeout = DEFAULT_TIMEOUT;
int hideUserAgent = 0;

CHttp::CHttp()
{
}

CHttp::~CHttp()
{
}


/*
* Actually downloads the page, registering a hit (donation)
* If the fileBuf passed in is NULL, the url is downloaded and then
* freed; otherwise the necessary space is allocated for fileBuf.
* Returns size of download on success, 
	-1 on error is set,
	-2 out of ip block,
	-3 invalid host,
	-4 MIME is imag/xxx
	-300 on 301.
 */
int CHttp::Fetch(string strUrl, char **fileBuf, char **fileHeadBuf, char **location, int* nPSock )
{
	char *tmp, *url, *requestBuf, *pageBuf;
	const char *host, *path;
	int sock, bytesRead = 0, bufsize = REQUEST_BUF_SIZE;
	int ret = -1, tempSize, selectRet;
	int port = 80;


	if( strUrl.empty() ){
		cout << "strUrl is NULL" << endl;
		return -1;
	}

	/* Copy the url passed in into a buffer we can work with, change, etc. */
/*
	url = (char*)malloc(strUrl.length()+1);
	if( url == NULL ){
		cout << "can not allocate enought memory for url" << endl;
		return -1;
	} else {
		memset(url, 0,strUrl.length()+1);
		memcpy(url, strUrl.c_str(), strUrl.length() );
	}
*/
	//pthread_mutex_lock(&mutexMemory);
	url = strdup(strUrl.c_str());
	//pthread_mutex_unlock(&mutexMemory);
	if( url == NULL ){
		cout << "!error: stdup() in Fetch()" << endl;
		return -1;
	}

	// parse the url
	CUrl u;
	if( u.ParseUrlEx(url) == false ){
		cout << "ParseUrlEx error in Fetch(): " << strUrl << endl;
		return -1;
	}

	host = u.m_sHost.c_str();
	path = u.m_sPath.c_str();
	if( u.m_nPort > 0 ) port = u.m_nPort;

	/* Compose a request string */
	//pthread_mutex_lock(&mutexMemory);
	requestBuf = (char*)malloc(bufsize);
	//pthread_mutex_unlock(&mutexMemory);
	if(requestBuf == NULL){
		if (url)
		{
			//pthread_mutex_lock(&mutexMemory);
			free(url); url=NULL;
			//pthread_mutex_unlock(&mutexMemory);
		}
		cout << "can not allocate enought memory for requestBuf" << endl;
		return -1;
	}
	requestBuf[0] = 0;

	if( strlen(path) < 1 ){
		/* The url has no '/' in it, assume the user is making a root-level
                 *      request */
		tempSize = strlen("GET /") + strlen(HTTP_VERSION) +2;
/*
		if( tempSize > bufsize ){
			free(url);
			free(requestBuf);
			cout << "tempSize larger than bufsize" << endl;
			return -1;
		}
*/

		if(checkBufSize(&requestBuf, &bufsize, tempSize) ||
			snprintf(requestBuf, bufsize, "GET / %s\r\n", 
			HTTP_VERSION) < 0 )
		{
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
				 free(url); url=NULL;
			}
			if (requestBuf)
			{
				 free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "1.checkBuffSize(&requestBuf..) error" << endl;
			return -1;
		}

	}else{
		tempSize = strlen("GET ") + strlen(path) + strlen(HTTP_VERSION) + 4;

		if(checkBufSize(&requestBuf, &bufsize, tempSize) ||
			snprintf(requestBuf, bufsize, "GET %s %s\r\n", 
			path, HTTP_VERSION) < 0)
		{

			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
				 free(url); url=NULL;
			}
			if (requestBuf)
			{
				 free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "2._checkBuffSize(&requestBuf..) error" << endl;
			return -1;
		}

	}


	/* Use Host: even though 1.0 doesn't specify it.  Some servers
         *      won't play nice if we don't send Host, and it shouldn't hurt anything */
	tempSize = (int)strlen("Host: ") + (int)strlen(host) + 3;/* +3 for "\r\n\0" */

	if(checkBufSize(&requestBuf, &bufsize, tempSize + 128)){
		//pthread_mutex_lock(&mutexMemory);
		if (url)
		{
			 free(url); url=NULL;
		}
		if (requestBuf)
		{
			 free(requestBuf); requestBuf=NULL;
		}
		//pthread_mutex_unlock(&mutexMemory);
		cout << "3._checkBuffSize(&requestBuf..) error" << endl;
		return -1;
	}

	strcat(requestBuf, "Host: ");
	strcat(requestBuf, host);
	strcat(requestBuf, "\r\n");

	if(!hideUserAgent && userAgent == NULL) {

		tempSize = (int)strlen("User-Agent: ") +
			(int)strlen(DEFAULT_USER_AGENT) + (int)strlen(VERSION) + 4;
		if(checkBufSize(&requestBuf, &bufsize, tempSize)) {
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
			 	free(url); url=NULL;
			}
			if (requestBuf)
			{
			 	free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "4._checkBuffSize(&requestBuf..) error" << endl;
			return -1;
		}
		strcat(requestBuf, "User-Agent: ");
		strcat(requestBuf, DEFAULT_USER_AGENT);
		strcat(requestBuf, "/");
		strcat(requestBuf, VERSION);
		strcat(requestBuf, "\r\n");

	} else if(!hideUserAgent) {

		tempSize = (int)strlen("User-Agent: ") + (int)strlen(userAgent) + 3;
		if(checkBufSize(&requestBuf, &bufsize, tempSize)) {

			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
			 	free(url); url=NULL;
			}
			if (requestBuf)
			{
			 	free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "5._checkBuffSize(&requestBuf..) error" << endl;
			return -1;
		}
		strcat(requestBuf, "User-Agent: ");
		strcat(requestBuf, userAgent);
		strcat(requestBuf, "\r\n");
	}

	//tempSize = (int)strlen("Connection: Close\n\n");
	tempSize = (int)strlen("Connection: Keep-Alive\r\n\r\n");
	if(checkBufSize(&requestBuf, &bufsize, tempSize)) {
		//pthread_mutex_lock(&mutexMemory);
		if (url)
		{
		 	free(url); url=NULL;
		}
		if (requestBuf)
		{
		 	free(requestBuf); requestBuf=NULL;
		}
		//pthread_mutex_unlock(&mutexMemory);
		cout << "6._checkBuffSize(&requestBuf..) error" << endl;
		return -1;
	}


	//strcat(requestBuf, "Connection: Close\n\n");
	strcat(requestBuf, "Connection: Keep-Alive\r\n\r\n");


	/* Now free any excess memory allocated to the buffer */
	//pthread_mutex_lock(&mutexMemory);
	tmp = (char *)realloc(requestBuf, strlen(requestBuf) + 1);
	//pthread_mutex_unlock(&mutexMemory);
	if(tmp == NULL){
		//pthread_mutex_lock(&mutexMemory);
		if (url)
		{
		 	free(url); url=NULL;
		}
		if (requestBuf)
		{
		 	free(requestBuf); requestBuf=NULL;
		}
		//pthread_mutex_unlock(&mutexMemory);
		cout << "realloc for tmp error" << endl;
		return -1;
	}
	requestBuf = tmp;

	if( *nPSock != -1 ){
		sock = *nPSock;
		cout << "using privous socket " << *nPSock << endl;
	}else{

		// cout << "1.get a new one" << endl;
		sock = CreateSocket( host, port );
		if(sock == -1) { // invalid host
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
		 		free(url); url=NULL;
			}
			if (requestBuf)
			{
		 		free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			return -3;
		}
		if(sock == -2) { // out of ip block
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
		 		free(url); url=NULL;
			}
			if (requestBuf)
			{
		 		free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			//cout << "2.not able to MakeSocket" << endl;
			return -2;
		}
	}
	


	ret = write(sock, requestBuf, strlen(requestBuf));
	if( ret == 0 ){
		cout << "requestBuf is " << requestBuf << endl;
		cout << "write nothing" << endl;
		//pthread_mutex_lock(&mutexMemory);
		if (url)
		{
			free(url); url=NULL;
		}
		if (requestBuf)
		{
			free(requestBuf); requestBuf=NULL;
		}
		//pthread_mutex_unlock(&mutexMemory);
		close(sock);
		*nPSock = -1;
		return -1;
		
	}
	if( ret == -1){
		//cout << "write error" << endl;
		// sock is invalid,we should make a new one
		close(sock);
		*nPSock  = -1;

		cout << "2.close previous socket " << *nPSock << " and get a new one" << endl;
		//maybe sock is dead,try again
		sock = CreateSocket( host, port );
		if(sock == -1) { 
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
				free(url); url=NULL;
			}
			if (requestBuf)
			{
				free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "3.not able to MakeSocket" << endl;
			return -1;
		}
		if(sock == -2) { 
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
				free(url); url=NULL;
			}
			if (requestBuf)
			{
				free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			cout << "4.not able to MakeSocket" << endl;
			return -1;
		}
		if(write(sock, requestBuf, strlen(requestBuf)) == -1){
			//pthread_mutex_lock(&mutexMemory);
			if (url)
			{
				free(url); url=NULL;
			}
			if (requestBuf)
			{
				free(requestBuf); requestBuf=NULL;
			}
			//pthread_mutex_unlock(&mutexMemory);
			close(sock);
			*nPSock = -1;
			cout << "write error" << endl;
			return -1;
		}
	}

	//pthread_mutex_lock(&mutexMemory);
	if (url)
	{
		free(url); url=NULL;
	}
	if (requestBuf)
	{
		free(requestBuf); requestBuf=NULL;
	}
	//pthread_mutex_unlock(&mutexMemory);


	char headerBuf[HEADER_BUF_SIZE];
	/* Grab enough of the response to get the metadata */
	memset( headerBuf,0,HEADER_BUF_SIZE );
	//cout << "old sock is " << sock << endl;
	ret = read_header(sock, headerBuf);
	//cout << "ret = " << ret << endl;
	if(ret < 0) { 
		close(sock); 
		*nPSock = -1;
		return -1;
	}

	//cout << headerBuf << endl;
	if( strlen(headerBuf) == 0 ){
		cout << "strlen(headerBuf) = 0" << headerBuf << endl;
		cout << "strUrl: " << strUrl << endl << endl;;
		close(sock);
        *nPSock = -1;
		return -1;
	}

	CPage iPage;
	iPage.ParseHeaderInfo(headerBuf);
	if (iPage.m_nStatusCode == -1)
	{
		close(sock);
		*nPSock = -1;
		cout << "headerBuf: " << headerBuf << endl;
		cout << "!header error: not find HTTP" << endl;
		return -1;
	}

#ifdef DEBUG	// http return code
	cout <<"######Http return code: ######" << endl << i << endl;
#endif


	// deal with http://net.cs.pku.edu.cn/~cnds
	if (iPage.m_nStatusCode == 301 || iPage.m_nStatusCode == 302)
	{
		if (iPage.m_sLocation.empty() || iPage.m_sLocation.size()>URL_LEN)
		{	
			close(sock);
			*nPSock = -1;
			cout << headerBuf << endl;
			cout << "!error: Location" << endl;
			return -1;
		} else{
			//pthread_mutex_lock(&mutexMemory);
			char *loc=strdup(iPage.m_sLocation.c_str());
			//pthread_mutex_unlock(&mutexMemory);
			*location = loc;
			close(sock);
			*nPSock = -1;
			return -300;
		}
	}

	if(iPage.m_nStatusCode<200 || iPage.m_nStatusCode>299 ){
		close(sock);
		*nPSock = -1;
		cout << "!header code = " << iPage.m_nStatusCode << endl;
		return -1;
	}

	// when crawling images for ImgSE, remember to comment the paragraph
	// when crawling plain text for SE, remember to open the paragraph
	// paragraph begin
	if( iPage.m_sContentType.find("image") != string::npos ){ // 
		close(sock);
		*nPSock = -1;
		return -4;
    }
	// paragraph end

	if (iPage.m_nContentLength == -1)
	{
		close(sock);
		*nPSock = -1;
		cout << headerBuf << endl;
		cout << "!error: Content-length" << endl;
		return -1;
	}

	if (iPage.m_nContentLength==0 || iPage.m_nContentLength<20)
	{ // Allocate enough memory to hold the page 
		iPage.m_nContentLength = DEFAULT_PAGE_BUF_SIZE;
	}


	if (iPage.m_nContentLength > MAX_PAGE_BUF_SIZE)
	{
		cout << "the page discarded due to its size " 
			<< iPage.m_nContentLength 
			<< " is larger than " << MAX_PAGE_BUF_SIZE << endl;
		close(sock);
		*nPSock = -1;
		return -1;
	}

#ifdef DEBUG	// http content length
	cout <<"######Content length: ######" << endl << iPage.m_nContentLength << endl;
#endif

	//pthread_mutex_lock(&mutexMemory);
	pageBuf = (char *)malloc(iPage.m_nContentLength);
	//pthread_mutex_unlock(&mutexMemory);
	if(pageBuf == NULL){
		close(sock);
		*nPSock = -1;
		cout << "malloc for pageBuf" << endl;
		return -1;
	}
	
        /* Begin reading the body of the file */
	fd_set rfds;
	struct timeval tv;
	int flags;

	flags=fcntl(sock,F_GETFL,0);
    if(flags<0){
		close(sock);
		*nPSock = -1;
		if (pageBuf)
		{
			//pthread_mutex_lock(&mutexMemory);
			free(pageBuf); pageBuf=NULL;
			//pthread_mutex_unlock(&mutexMemory);
		}
		cout << "1.fcntl() error " << endl;
		return -1;
	}
	
    flags|=O_NONBLOCK;
    if(fcntl(sock,F_SETFL,flags)<0){
		close(sock);
		*nPSock = -1;
		if (pageBuf)
		{
			free(pageBuf); pageBuf=NULL;
		}
		cout << "2.fcntl() error " << endl;
		return -1;
	}

	int pre_ret=0;
	while(ret > 0){
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		if( bytesRead == iPage.m_nContentLength ){
			tv.tv_sec = 1;
		}else{
			tv.tv_sec = timeout;
		}
		tv.tv_usec = 0;

		if(DEFAULT_TIMEOUT >= 0)
			selectRet = select(sock+1, &rfds, NULL, NULL, &tv);
		else            /* No timeout, can block indefinately */
			selectRet = select(sock+1, &rfds, NULL, NULL, NULL);

		if(selectRet == 0 && timeout < 0){
			close(sock);
			*nPSock = -1;
			if (pageBuf)
			{
				//pthread_mutex_lock(&mutexMemory);
				free(pageBuf); pageBuf=NULL;
				//pthread_mutex_unlock(&mutexMemory);
			}
			cout << "selectRet == 0 && timeout < 0" << endl;
			return -1;
		} else if(selectRet == -1){
			close(sock);
			*nPSock = -1;
			if (pageBuf)
			{
				//pthread_mutex_lock(&mutexMemory);
				free(pageBuf); pageBuf=NULL;
				//pthread_mutex_unlock(&mutexMemory);
			}
			cout << "selectRet == -1" << endl;
			return -1;
		}

                ret = read(sock, pageBuf + bytesRead, iPage.m_nContentLength);
                //ret = read(sock, (char*)pageBuf.c_str() + bytesRead, iPage.m_nContentLength);

		if(ret == 0) break;
		if(ret == -1 && pre_ret==0) {
			close(sock);
			*nPSock = -1;
			if (pageBuf)
			{
				//pthread_mutex_lock(&mutexMemory);
				free(pageBuf); pageBuf=NULL;
				//pthread_mutex_unlock(&mutexMemory);
			}
			cout << "read()'s retval=-1" << endl;
			return -1;
		}else if( ret == -1 && pre_ret ){
			//cout << "2. pre_ret = " << pre_ret << endl;
/*
			if( bytesRead < iPage.m_nContentLength){	// meaning we lost the connection too soon
				cout << "lost the connection too soon" << endl;
				freeOpageBuf);
				return -1;
			}
*/
			break;
		}

		pre_ret = ret;
		//cout << "1.pre_ret = " << pre_ret << endl;

		bytesRead += ret;


			/* To be tolerant of inaccurate Content-Length fields, we'll
			 *      allocate another read-sized chunk to make sure we have
			 *      enough room.
			 */
		if(ret > 0) {
			//pthread_mutex_lock(&mutexMemory);
			pageBuf = (char *)realloc(pageBuf, bytesRead + iPage.m_nContentLength);
			//pthread_mutex_unlock(&mutexMemory);
			if(pageBuf == NULL) {
				close(sock);
				*nPSock = -1;
				if (pageBuf)
				{
					//pthread_mutex_lock(&mutexMemory);
					free(pageBuf); pageBuf=NULL;
					//pthread_mutex_unlock(&mutexMemory);
				}
				cout << "realloc()" << endl;
				return -1;
			}
		}

	}

	/*
	 * The download buffer is too large.  Trim off the safety padding.
	*/

	//pthread_mutex_lock(&mutexMemory);
	pageBuf = (char *)realloc(pageBuf, bytesRead+1);
	//pthread_mutex_unlock(&mutexMemory);
	if(pageBuf == NULL){
		close(sock);
		*nPSock = -1;
		if (pageBuf)
		{
			//pthread_mutex_lock(&mutexMemory);
			free(pageBuf); pageBuf=NULL;
			//pthread_mutex_unlock(&mutexMemory);
		}
		cout << "2.realloc()" << endl;
		return -1;
	}


	pageBuf[bytesRead] = '\0';


	if(fileBuf == NULL){	/* They just wanted us to "hit" the url */
		if (pageBuf)
		{
			//pthread_mutex_lock(&mutexMemory);
			free(pageBuf); pageBuf=NULL;
			//pthread_mutex_unlock(&mutexMemory);
		}
	}else{



		char *tmp;
		//tmp = (char *)malloc(HEADER_BUF_SIZE);
		//pthread_mutex_lock(&mutexMemory);
		tmp = (char *)malloc(strlen(headerBuf)+1);
		//pthread_mutex_unlock(&mutexMemory);
        	if(tmp == NULL){
                	close(sock);
			*nPSock = -1;
			if (pageBuf)
			{
				//pthread_mutex_lock(&mutexMemory);
				free(pageBuf); pageBuf=NULL;
				//pthread_mutex_unlock(&mutexMemory);
			}
			cout << "malloc() for headerBuf" << endl;
                	return -1;
        	}
		//memcpy( tmp, headerBuf, HEADER_BUF_SIZE-1 );
		strncpy( tmp, headerBuf, strlen(headerBuf)+1 );
		*fileHeadBuf = tmp;

		*fileBuf = pageBuf;
	}
		
	//close(sock);
	*nPSock = sock;
	return bytesRead;
}
	
int CHttp::CreateSocket(const char *host, int port)
{
	int sock;		// Socket descriptor
	struct sockaddr_in sa;	// Socket address


	unsigned long   inaddr;
	int ret;

	CUrl url;
	char *ip = url.GetIpByHost(host);

	if( ip == NULL ){ // gethostbyname() error in GetIpByHost()
		//cout << "invalid host: " << host << endl;
		return -1;

	} else {
		// filter ip (decide whether it is inside the ip block)
		if( url.IsValidIp(ip) ){
			// inside
			inaddr = (unsigned long)inet_addr(ip);

			if( inaddr == INADDR_NONE ){
				// release the buffer, be careful
				//pthread_mutex_lock(&mutexMemory);
				delete [] ip; ip = NULL;
				//pthread_mutex_unlock(&mutexMemory);
				cout << "invalid ip " << ip << endl;
				return -1;
			}

			memcpy((char *)&sa.sin_addr, (char *)&inaddr, sizeof(inaddr));

			// release the buffer, be carful
			//pthread_mutex_lock(&mutexMemory);
			delete [] ip; ip = NULL;
			//pthread_mutex_unlock(&mutexMemory);

		} else { // out of ip block
			// release the buffer, be carful
			//pthread_mutex_lock(&mutexMemory);
			delete [] ip; ip = NULL;
			//pthread_mutex_unlock(&mutexMemory);
			//cout << "out of ip block: " << host << endl;
			return -2;
		}
	}


	/* Copy host address from hostent to (server) socket address */
	sa.sin_family = AF_INET;		
	sa.sin_port = htons(port);	/* Put portnum into sockaddr */

	sock = -1;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0 ) { 
		cout << "socket() in CreateSocket" << endl;
		return -1;
	}

	int optval = 1;
  	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
		(char *)&optval, sizeof (optval)) < 0){

		cout << "setsockopt() in CreateSocket" << endl;
		close(sock);
		return -1;
	}

    //ret = connect(sock, (struct sockaddr *)&sa, sizeof(sa));
    ret = nonb_connect(sock, (struct sockaddr *)&sa, DEFAULT_TIMEOUT);
    if(ret == -1) { 
		cout << "nonb_connect() in CreateSocket" << endl;
		close(sock);
		return -1; 
	}

    return sock;
}
		

int CHttp::read_header(int sock, char *headerPtr)
{
	fd_set rfds;
	struct timeval tv;
	int bytesRead = 0, newlines = 0, ret, selectRet;

	int flags;

	flags=fcntl(sock,F_GETFL,0);
	if(flags<0){
		cout << "1.fcntl() in read_header()< 0" << endl;
		return -1;
	}

        flags|=O_NONBLOCK;
        if(fcntl(sock,F_SETFL,flags)<0){
		cout << "2.fcntl() < 0 in read_header()" << endl;
		return -1;
	}

	while(newlines != 2 && bytesRead != HEADER_BUF_SIZE-1) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		if(timeout >= 0)
			selectRet = select(sock+1, &rfds, NULL, NULL, &tv);
		else            /* No timeout, can block indefinately */
			selectRet = select(sock+1, &rfds, NULL, NULL, NULL);

		if(selectRet == 0 && timeout < 0) {
			cout << "selectRet == 0 && timeout < 0" << endl;
			return -1;
                }else if(selectRet == -1) {
			cout << "selectRet == 0 && timeout < 0 else" << endl;
			return -1;
		}
                
		ret = read(sock, headerPtr, 1);
                if(ret == -1){
			cout << "!error: read() in read_header()" << endl;
			return -1;
		}

		bytesRead++;
                
		if(*headerPtr == '\r') {                 /* Ignore CR */
			/* Basically do nothing special, just don't set newlines
			 *      to 0 */
			headerPtr++;
			continue;
		}
		else if(*headerPtr == '\n')             /* LF is the separator */
			newlines++;
		else    
			newlines = 0;
                
		headerPtr++;

	}
        
	//headerPtr -= 3;         /* Snip the trailing LF's */
				  /* to be compatible with Tianwang format, we have to retain them*/
	headerPtr -= 2;
	*headerPtr = '\0';
	//cout << "in it " << headerPtr << endl;
	return bytesRead;
}

/*
 *function nonblocking connect
 *parameter sec is the second of timing out
 */

int CHttp::nonb_connect(int sockfd,struct sockaddr* sa,int sec)
{
	int flags;
	int status;
	fd_set mask;
	struct timeval timeout;

	//set the socket as nonblocking
	flags=fcntl(sockfd,F_GETFL,0);
	if(flags<0) return -1;
	flags|=O_NONBLOCK;
	if(fcntl(sockfd,F_SETFL,flags) < 0){
		cout << "1.fcntl() in nonb_connect" << endl;
		return -1;
	}

	if( connect(sockfd,sa,sizeof(struct sockaddr)) == 0){
		flags&=~O_NONBLOCK;
		fcntl(sockfd,F_SETFL,flags);
		return sockfd;//connected immediately
        }

	FD_ZERO(&mask);
	FD_SET(sockfd,&mask);
	timeout.tv_sec=sec;
	timeout.tv_usec=0;
	status=select(sockfd+1,NULL,&mask,NULL,&timeout);

	switch(status){
		case -1:		// Select error, set the socket as default blocking
			flags&=~O_NONBLOCK;
			fcntl(sockfd,F_SETFL,flags);
			cout << "2.fcntl() in nonb_connect" << endl;
			return -1;
		case 0: 		//Connection timed out.
			flags&=~O_NONBLOCK;
			fcntl(sockfd,F_SETFL,flags);
			cout << "3.fcntl() in nonb_connect" << endl;
			return -1;
		default: 		// Connected successfully.
			FD_CLR(sockfd,&mask);
			flags&=~O_NONBLOCK;
			fcntl(sockfd,F_SETFL,flags);
			return 0;
	}
}

/*
 * Determines if the given NULL-terminated buffer is large enough to
 *      concatenate the given number of characters.  If not, it attempts to
 *      grow the buffer to fit.
 * Returns:
 *      0 on success, or
 *      -1 on error (original buffer is unchanged).
 */
int CHttp::checkBufSize(char **buf, int *bufsize, int more)
{
	char *tmp;
	int roomLeft = *bufsize - (strlen(*buf) + 1);

	if(roomLeft > more) return 0;

	//pthread_mutex_lock(&mutexMemory);
	tmp = (char *)realloc(*buf, *bufsize + more + 1);
	//pthread_mutex_unlock(&mutexMemory);
	if(tmp == NULL) return -1;

	*buf = tmp;
	*bufsize += more + 1;
	return 0;
}
