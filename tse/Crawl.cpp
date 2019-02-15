#include "Crawl.h"
#include "Url.h"
#include "Md5.h"

#include <list.h>
#include <hlink.h>
#include <uri.h>

extern pthread_mutex_t mymutex;
extern map<string,string> mapCacheHostLookup;
extern vector<string> vsUnreachHost;
extern char **ParseRobot( char *data, char len);

set<string> setVisitedUrlMD5;
set<string> setVisitedPageMD5;
set<string> setUnvisitedUrlMD5;
set<string> setUnreachHostMD5;

//LB_c: 处理重复网页时用，first为page内容的MD5值，second为page对应的url字符串
multimap<string, string, less<string> > replicas;

pthread_mutex_t mutexCollection = PTHREAD_MUTEX_INITIALIZER;	// unvisited urls
pthread_mutex_t mutexUnreachHost = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexUnvisitedUrlMD5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVisitedUrlMD5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVisitedPageMD5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDetect = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexLink4SEFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexLink4HistoryFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIsamFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVisitedUrlFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexUnreachHostFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexReplicas = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutexMemory = PTHREAD_MUTEX_INITIALIZER;

//LB_c: 该数据的格式说明见 CCrawl::GetIpBlock()
map<unsigned long,unsigned long> mapIpBlock;
//LB_c: 标记抓取过程的状态，抓取线程通过判断该标志来结束抓取
bool b_fOver;
//LB_c: 存储在内存中的待抓取url记录，每条记录第一项为url的host(主机名)，第二项为url字符串
multimap<string,string > mmapUrls;

typedef map<unsigned long,unsigned long>::value_type valTypeIpBlock;
typedef map<string,string>::value_type mvalType;

void SaveReplicas(const char* filename);

struct package
{
	CCrawl *crawl;
	CPage *page;
};

//LB_c: 存储当前页面中提取出的url
vector<string> vsParsedLinks;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LB_c: onfind是一个回调函数，hlink_detect_string函数中每找到一个url便调用该回调函数，
// 将url加入到待访问的集合mmapUrls，并加入vsParsedLinks中。参数uri便是提取出的url信息；参数elem应该
// 表示该url的类型，如是不是图片链接，等价于CUrl类的IsImageUrl函数的功能。
int onfind(const char *elem, const char *attr, struct uri *uri, void *arg)
{
	struct package *p=(struct package*)arg;
	char buff[URL_LEN+1];
	
	//LB_c: 该函数是将struct uri形式的url信息重新组装成字符串形式，存储到buff
	if (uri_recombine(uri, buff, URL_LEN+1, C_SCHEME| C_AUTHORITY| C_PATH| C_QUERY ) >= 0)
	{
		//LB_c: 将字符串形式的url(buff)存入vsParsedLinks
		vsParsedLinks.push_back(buff);
		if( !p->page->IsFilterLink(buff) )
		{
			// accept "a,link,frame,iframe,img,area"
			
			if (strcasecmp(elem, "img") == 0)
			{
				pthread_mutex_lock(&mutexLink4HistoryFile);
				if( p->crawl->m_ofsLink4HistoryFile ){
					p->crawl->m_ofsLink4HistoryFile << buff << endl;
				}
				pthread_mutex_unlock(&mutexLink4HistoryFile);
			} else {
				//LB_c: 这里很关键，如果提取出的url是一个有效的链接，则要加入到待抓取集合mmapUrls中，
				// 抓取线程不断从该集合中取出url进行抓取。
				p->crawl->AddUrl( buff );
			}
		}
	}

	uri_destroy(uri);
	free(uri);
	return 1;
}

/***********************************************************************
 * Function name: start
 * Input argv:
 * 	-- arg: the CCrawl handle
 * Output argv:
 * 	--
 * Return:
***********************************************************************/
void* start(void *arg)
{
	( (CCrawl*)arg )->fetch(arg);
}

/*****************************************************************
 * Function name: SaveUnvisitedUrl
 * Input argv:
 *      --
 * Output argv:
 *      --
 * Return:
 * Function Description: Save teh Unvisited Url
 * Version: 1.0
 * Be careful:
 ****************************************************************/
void SaveUnvisitedUrl()
{
	ofstream ofsUnvisitedUrl;
	ofsUnvisitedUrl.open(UNVISITED_FILE.c_str(), ios::in|ios::out|ios::trunc|ios::binary);
        if (!ofsUnvisitedUrl) {
		cerr << "cannot open " << UNVISITED_FILE << "for output" << endl;
		exit (-1);
	}

	multimap<string,string>::iterator it = mmapUrls.begin();
	for (; it!=mmapUrls.end(); it++) {
		ofsUnvisitedUrl << ((*it).second).c_str() << "\n";
	}

	ofsUnvisitedUrl << endl;
	ofsUnvisitedUrl.close();
}

/***********************************************************************
 * Function name: fetch
 * Input argv:
 * 	-- arg: the CCrawl handle
 * Output argv:
 * 	--
 * Return:
***********************************************************************/
void CCrawl::fetch(void *arg)
{
	string strUrl,host;

	int	nGSock = -1;
	string	strGHost = "";

	// create a Tianwang file for output the raw page data
	string ofsName = DATA_TIANWANG_FILE + "." + CStrFun::itos(pthread_self());
	CTianwangFile tianwangFile(ofsName);

	// create a Link4SE file for output the raw link data
	ofsName = DATA_LINK4SE_FILE + "." + CStrFun::itos(pthread_self());
	CLink4SEFile link4SEFile(ofsName);

	int iSleepCnt=0;
	for(;;){
		pthread_mutex_lock(&mutexCollection);
		int cnt = mmapUrls.size();
		if(cnt > 0){
			cout << "collection has: " << cnt << " unvisited urls" << endl;
			multimap<string,string>::iterator it=mmapUrls.begin();
			if( it != mmapUrls.end() ){
				// get an URL
				strUrl = (*it).second;

				// remove it from the collection
				mmapUrls.erase( it );

				pthread_mutex_unlock(&mutexCollection);

				// parse URL
				CUrl iUrl;
				if( iUrl.ParseUrlEx(strUrl) == false ){
					cout << "ParseUrlEx error in fetch(): " << strUrl << endl;
					continue;
				}

				if( strGHost != iUrl.m_sHost ){
					close( nGSock );
					nGSock = -1;
					strGHost = iUrl.m_sHost;
				}

				(( CCrawl* )arg)->DownloadFile(&tianwangFile, &link4SEFile, iUrl, nGSock);

				cnt = 0;
			} else {
				pthread_mutex_unlock(&mutexCollection);
			}
		} else {
			pthread_mutex_unlock(&mutexCollection);
			usleep(1000);
			iSleepCnt++;
		}

		if( b_fOver == true && iSleepCnt==200)
			break;
		
	}

	tianwangFile.Close();
	link4SEFile.Close();
}

/***********************************************************************
 * Function name: DownloadFile
 * Input argv:
 * 	-- pTianwang: the CCrawl handle
 * 	-- pLink4SE: the CCrawl handle
 * 	-- iUrl: the URL for crawling
 * 	-- nGSock: the previous global socket
 * Output argv:
 * 	--
 * Return:
***********************************************************************/
void CCrawl::DownloadFile(CTianwangFile *pTianwangFile,
	CLink4SEFile *pLink4SEFile, CUrl iUrl, int& nGSock)
{
	char *downloaded_file = NULL, *fileHead = NULL, *location = NULL;
	int file_length = 0;
	string strUrlLocation = "";
	//LB_c: nGSock是传进来的sock，记录上一次的socket连接，nSock是临时的
	int nSock = nGSock;

	cout << "1. pid=" << pthread_self() << " sock = " << nGSock << endl;

	//LB_c: CHttp是http协议的关键处理类
	CHttp http;
	//LB_c: Fetch是CHttp类的关键重要接口，该函数根据url进行http连接并下载网页
	file_length = http.Fetch(iUrl.m_sUrl, &downloaded_file, &fileHead, &location, &nSock);

#ifdef DEBUG	// just download
	cout << "######file length: ######" << file_length << endl;
	cout << "######head: ######" << fileHead << endl;
#endif

	//========================================= LB_c: http.Fetch返回-300，处理URL跳转 =======================================
	int nCount = 0;
	while( file_length == -300 ){ // moved to an another place
		//LB_c: 如果跳转的地址location长度非法，或者连续跳转3次，则停止处理
		if( strlen(location) > URL_LEN-1 || nCount == 3 || strlen(location)==0 ){
			if( location )
			{
				//pthread_mutex_lock(&mutexMemory); 
				free( location ); location = NULL;
				//pthread_mutex_unlock(&mutexMemory);
			}
			file_length = -1;
			break;
		}

		strUrlLocation = location;
		if(location)
		{
			//pthread_mutex_lock(&mutexMemory);
			free(location); location = NULL;
			//pthread_mutex_unlock(&mutexMemory);
		}

		string::size_type idx1 = CStrFun::FindCase(strUrlLocation, "http");
		//LB_c: idx1!=0说明strUrlLocation开头不是"http"，不含http说明是同一个host内部跳转，
		// 根据iUrl和strUrlLocation构造出跳转的url。 idx == 0说明strUrlLocation头部是"http"，
		// 说明是一个完整的url，则不需要特殊处理。
		if( idx1 != 0 ){
			char c1 = iUrl.m_sUrl.at(iUrl.m_sUrl.length()-1);
			char c2 = strUrlLocation.at(0);

			if( c2 == '/' ){
				strUrlLocation = "http://" + iUrl.m_sHost + strUrlLocation;
			}else if(  c1!='/' && c2!='/'){
				string::size_type idx;
                                                                                                        
                idx = iUrl.m_sUrl.rfind('/');
                if( idx != string::npos ){
                        if( idx > 6 ){ // > strlen("http://..")
                            strUrlLocation = iUrl.m_sUrl.substr(0, idx+1) + strUrlLocation;
                        } else {
                            strUrlLocation = iUrl.m_sUrl + "/" + strUrlLocation;
                        }                                                                                       
                } else {
					file_length = -1;
					break;
				}
			} else {
				if( c1=='/' ){
                        strUrlLocation = iUrl.m_sUrl + strUrlLocation;
                } else {
                        strUrlLocation = iUrl.m_sUrl + "/" + strUrlLocation;
                }
			}
		}

		CPage iPage;
		//LB_c: 过滤作弊网页
		if( iPage.IsFilterLink(strUrlLocation) ){
			file_length = -1;
			break;
		}

		cout << "2. pid=" << pthread_self() << " sock = " << nGSock << endl;
		//LB_c: 重新下载跳转的网页
		file_length = http.Fetch( strUrlLocation, &downloaded_file, &fileHead, &location, &nSock);
		nCount++;
	}
	//=======================================================================================================================

	//LB_c: 保存新的连接nSock到nGSock
	nGSock = nSock;

	//================================== LB_c: http.Fetch返回-1,出错，释放临时空间返回 ======================================
	if(file_length == -1){ // unreachable, skipped.
		cout << "!-: " << iUrl.m_sUrl << endl;
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}
		cout << "-unreach host: " << iUrl.m_sHost << endl;;
		return;
	}
	//=======================================================================================================================

	//============================= LB_c: http.Fetch返回-2,out of ip block，释放临时空间返回 ================================
	if(file_length == -2){ // out of ip block .
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}

		//LB_?: 当前url不在ip block中，是否要将host加入UnreachHost!? 因为不是unreach。下次变换了ipblock是否会有影响?
		// save unreach host
        SaveUnreachHost(iUrl.m_sHost);
		cout << "-out of block host: " << iUrl.m_sHost << endl;;
		return;
	}
	//=======================================================================================================================
	
	//=========================== LB_c: http.Fetch返回-3,invalid host or ip，释放临时空间返回 ===============================
	if(file_length == -3) { // invalid host or ip
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}
		
		//LB_?: 貌似这里需要将host加入UnreachHost，因为当前ip或host是无效的
		
		cout << "-invalid host: " << iUrl.m_sHost << endl;
		return;
	}
	//=======================================================================================================================

	//============================ LB_c: http.Fetch返回-3,当前url是图片链接，释放临时空间返回 ===============================
	if(file_length == -4) {	// MIME is image/xxx
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}

		if( m_ofsLink4HistoryFile ){
			pthread_mutex_lock(&mutexLink4HistoryFile);
			m_ofsLink4HistoryFile << iUrl.m_sUrl << endl;;
			pthread_mutex_unlock(&mutexLink4HistoryFile);
		}

		cout << "-imgage host: " << iUrl.m_sHost << endl;
		return;
	}
	//=======================================================================================================================


	
	// deal with normal page

	//============== LB_c: 网页头或者网页body数据为空则为异常，释放临时空间，返回 ==============
	if (!fileHead || !downloaded_file)
	{
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}
		close(nGSock);
		nGSock = -1;
		cout << "-size0 host: " << iUrl.m_sHost << endl;
		return;
	}
	//==========================================================================================

	//==================================== LB_c: 构造CPage类对象，进行解析 ====================================
	CPage iPage(iUrl.m_sUrl, strUrlLocation, fileHead, downloaded_file, file_length);
	if (fileHead)
	{
		free(fileHead); fileHead=NULL;
	}
	if (downloaded_file)
	{
		free(downloaded_file); downloaded_file=NULL;
	}

	//LB_c: 解析网页头信息，结果保存到CPage类成员中
	iPage.ParseHeaderInfo(iPage.m_sHeader);

	//LB_c: 服务器是否支持保持连接，否则关闭socket
	if( iPage.m_bConnectionState == false ){
		close(nGSock);
		nGSock = -1;
	}
	//=========================================================================================================
	

	//======================= LB_c: 这一段是检查网页的内容类型是否正确 =======================
	// when crawling images for ImgSE, remember to comment the paragraph
	// when crawling plain text for SE, remember to open the paragraph
	// paragraph begin
	if( iPage.m_sContentType != "text/html" && 
		iPage.m_sContentType != "text/plain" &&
		iPage.m_sContentType != "text/xml" &&
		iPage.m_sContentType != "application/msword" &&
		iPage.m_sContentType != "application/pdf" &&
		iPage.m_sContentType != "text/rtf" &&
		iPage.m_sContentType != "application/postscript" &&
		iPage.m_sContentType != "application/vnd.ms-execl" &&
		iPage.m_sContentType != "application/vnd.ms-powerpoint" ){

		cout << "-unwant type  host: " << iUrl.m_sHost << endl;
		return;
	}
	// paragraph end
	//=========================================================================================
	

#ifdef DEBUG    // content encoding
	cout <<"######Content encoding: ######" << endl 
		<< iPage.m_sContentEncoding << endl;
#endif


	//================================== LB_c: 这部分是解压缩gzip方式压缩的网页内容 ==================================
	char sUnzipContent[1024000];
	int  nUnzipLength = 0;
	if( iPage.m_sContentEncoding == "gzip" 
		&& iPage.m_sContentType == "text/html" ){

		gzFile zip;  
		string ofsGzipName;

		ofsGzipName = CStrFun::itos(pthread_self()) + ".gz";
		ofstream ofsDownloadFile(ofsGzipName.c_str(),ios::trunc | ios::binary);
		cout << "file_length: " << file_length << endl;
               	ofsDownloadFile.write(iPage.m_sContent.c_str(), iPage.m_nLenContent);
		ofsDownloadFile.close();

		zip = gzopen(ofsGzipName.c_str(),"rb");  
		if( zip == NULL ){
			cout << "Open zip file " << ofsGzipName.c_str() << " error." << endl;
			exit(-1);
		}
		nUnzipLength = gzread(zip, sUnzipContent, 1024000);
		if( nUnzipLength == -1 ){
			cout << "Read zip file " << ofsGzipName.c_str() << " error." << endl;
			exit(-1);
		}

   
		sUnzipContent[nUnzipLength]=0;

		gzclose(zip); 

	}
	//================================================================================================================


	//======= LB_c: 这部分是将当前的url的MD5值加入到setVisitedUrlMD5，并写入文件tse_md5.visitedurl中 =======
	CMD5 iMD5;
	string strDigest;

	/////////////////////////////
	// because we can make sure the url in the setVisitedUrlMd5
	// is not same(we have check it before insert it to the collection),
	// we intert it directly.  however...
	//iMD5.GenerateMD5( (unsigned char*)iPage.m_sUrl.c_str(), iPage.m_sUrl.length() );
	iMD5.GenerateMD5( (unsigned char*)iUrl.m_sUrl.c_str(), iUrl.m_sUrl.length() );
	strDigest = iMD5.ToString();
	
	//LB_c: 再次检查该url的MD5值是否已经在集合setVisitedUrlMD5，在说明已经访问过，返回!
	pthread_mutex_lock(&mutexVisitedUrlMD5);
	if( setVisitedUrlMD5.find(strDigest) != setVisitedUrlMD5.end() ) {
		cout << "!vurl: ";    //1.crawled already
		pthread_mutex_unlock(&mutexVisitedUrlMD5);
		return;
	}
		
	setVisitedUrlMD5.insert(strDigest);
	SaveVisitedUrlMD5(strDigest);
	pthread_mutex_unlock(&mutexVisitedUrlMD5);
	//=========================================================================================================


	//================================================================================================================
	//LB_c: 这一段处理重复的网页内容，注意是重复的网页内容而不是重复的url，即不同的url的内容完全一样。 个人觉得这里
	// 处理的意义不大，因为这里是通过对整个网页的内容求MD5值来判断重复的，所以两个网页只要有一个字符不同，两个网页
	// 的MD5值也会不同，因此不是重复网页。关于判重处理，有simhash方法还不错!
	/////////////////////////////
	// whether it is a visited page
	// for ImgSE, should comment this paragraph
	// for SE, should uncomment this paragraph
	// begin
	iMD5.GenerateMD5( (unsigned char*)iPage.m_sContent.c_str(), iPage.m_sContent.length() );
	strDigest = iMD5.ToString();
	pthread_mutex_lock(&mutexVisitedPageMD5);
	replicas.insert(pair<string, string>(strDigest, iPage.m_sUrl));
	if( setVisitedPageMD5.find(strDigest) != setVisitedPageMD5.end() ) {
		cout << "!vpage: ";		// crawled already
		pthread_mutex_unlock(&mutexVisitedPageMD5);
		return;
	}
	setVisitedPageMD5.insert(strDigest);
	SaveVisitedPageMD5(strDigest);
	pthread_mutex_unlock(&mutexVisitedPageMD5);		
	// end
	//================================================================================================================

	///////////////////////
	// save as ISAM file
	//SaveIsamRawData(&iUrl, &iPage);
	
	cout << "+";


	//LB_c: 将网页内容及相关数据写入天网格式文件 ============================
	////////////////////
	// save as Tianwang format
	SaveTianwangRawData(pTianwangFile, &iUrl, &iPage);
	//=======================================================================


	//=======================================================================
	//LB_c: 该url访问成功，将url记录到文件中，如果没有跳转记录原始url，有
	//跳转则记录实际地址m_sLocation。LB_?: 按理说这里的原始url也算被抓取
	//了，为什么不也添加到文件中呢?
	////////////////////
	// save visited Urls
	if( iPage.m_sLocation.length() < 1 ){
		SaveVisitedUrl(iUrl.m_sUrl);
	} else {
		SaveVisitedUrl(iPage.m_sLocation);
	}
	//=======================================================================

	//return;	// just crawl seeds


	//================================= LB_c: 以下是从页面(iPage)中提取url ===================================

	//LB_c: 只支持text/html格式的网页中提取url
	/////////////////////////////////////
	// Parse hyperlinks
	if (iPage.m_sContentType != "text/html") { // we can only find links in tex/html
		return;
	}
	// using XIE Han's link parser

    struct uri page_uri;

	pthread_mutex_lock(&mutexDetect);

	//--------------------------------- LB_c: url解析 --------------------------------------------
	if (iPage.m_sLocation.empty())
	{
		//LB_?: 该函数没找到源码
		//LB_c: C语言实现的url解析，功能与CUrl相同，<<搜索>>书中有说明，是为了兼容性一致性。
		// 将参数1的url字符串解析成url结构体，即解析出url的各字段，见struct uri定义。
		uri_parse_string(iPage.m_sUrl.c_str(), &page_uri);
	}
	else
	{
		uri_parse_string(iPage.m_sLocation.c_str(), &page_uri);
	}
	//--------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------
	//LB_c: 页面分析，提取urls(出链)，onfind是一个回调函数，hlink_detect_string函数中每找到一个
	//url便调用该回调函数，将url加入到待访问的集合mmapUrls，并加入vsParsedLinks中待下一步
	//SaveLink4SE031121函数中写入文件中。	
	struct package p={this, &iPage};
	hlink_detect_string(iPage.m_sContent.c_str(), &page_uri, onfind, &p);
	//--------------------------------------------------------------------------------------------

	//-----------------------------------------------------------------------
	//LB_c: 将当前页面中提取出的urls(出链)信息记录到文件link4SE.url中 ---
	struct file_arg pLinks = {&iUrl, &iPage};
	SaveLink4SE031121( &pLinks );
	//-----------------------------------------------------------------------

	// save as Link4SE format
	//SaveLink4SERawData(pLink4SEFile, &iUrl, &iPage);

	pthread_mutex_unlock(&mutexDetect);

	//LB_c: 这里的page_uri已经在onfind释放过了
	uri_destroy(&page_uri);
	cout << "Parse End......" << endl;

	//======================================================================================================
	
	return;
}


void SaveReplicas(const char* filename)
{
	//ofstream ofs(filename, ios::out|ios::app);
	ofstream ofs(filename, ios::out|ios::binary|ios::app);
	if( !ofs ){
		cout << "error open file " << endl;
	}
	string md5;

	pthread_mutex_lock(&mutexReplicas);
	multimap<string, string, less<string> >::const_iterator it;
	ostringstream *oss = 0;
	int i = 0;
	for ( it=replicas.begin(); it != replicas.end(); it ++)
	{
		if (!md5.empty() && md5 != it->first)
		{
			if (i>=2)
				ofs<<(*oss).str()<<endl;
			//pthread_mutex_lock(&mutexMemory);
			delete(oss);
			oss = new ostringstream;
			//pthread_mutex_unlock(&mutexMemory);
			(*oss)<<it->first<<endl;
			i = 0;
			md5 = it->first;
		}
		else if (md5.empty())
		{	
			md5 = it->first;
			//pthread_mutex_lock(&mutexMemory);
			oss = new ostringstream;
			//pthread_mutex_unlock(&mutexMemory);
			(*oss)<<it->first<<endl;
			i = 0;
		}
		if (oss != 0)
			(*oss)<<it->second<<endl;
		i++;
	}

	pthread_mutex_unlock(&mutexReplicas);
}


////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////////////

CCrawl::CCrawl()
{
}

//LB_c: inputFileName为url种子文件，即初始的待爬取网页的url
// outputFileName为爬取记录文件，记录所有已经爬取的url
CCrawl::CCrawl(string inputFileName, string outputFileName)
{
	m_sInputFileName = inputFileName;
	m_sOutputFileName = outputFileName; 
}

CCrawl::~CCrawl()
{
	m_ofsVisitedUrlFile.close();
	m_ofsLink4SEFile.close();
	m_ofsLink4HistoryFile.close();
	m_isamFile.Close();
	m_ofsVisitedUrlMD5File.close();
	m_ofsVisitedPageMD5File.close();
}

/*****************************************************************
 ** Function name: SigTerm
 ** Input argv:
 **      --
 ** Output argv:
 **      --
 ** Return:
 ** Function Description: signal function
 ** Version: 1.0
 ** Be careful:
 *****************************************************************/
static void SigTerm(int x)
{
	SaveUnvisitedUrl();
	SaveReplicas("repli");

	cout << "Terminated!" << endl;
	exit(0);
}

void CCrawl::GetVisitedUrlMD5()
{
	//LB_c: URL_MD5_FILE = "tse_md5.visitedurl"
	ifstream ifsMD5(URL_MD5_FILE.c_str(),ios::binary);
	if(!ifsMD5){
		//cerr << "did not find " << UrlMD5_FILE << " for iutput" << endl;
		return;
	}
	
	string strMD5;
	while( getline(ifsMD5,strMD5) ){
		setVisitedUrlMD5.insert(strMD5);	
	}

	ifsMD5.close();
	cout << "got " << setVisitedUrlMD5.size() << " md5 values of visited urls" << endl;
}

void CCrawl::GetVisitedPageMD5()
{
	//LB_c: PAGE_MD5_FILE = "tse_md5.visitedpage"
	ifstream ifsMD5(PAGE_MD5_FILE.c_str(),ios::binary);
	if(!ifsMD5){
		//cerr << "did not find " << PageMD5_FILE << " for iutput" << endl;
		return;
	}
	
	string strMD5;
	while( getline(ifsMD5,strMD5) ){
		setVisitedPageMD5.insert(strMD5);	
	}

	ifsMD5.close();
	cout << "got " << setVisitedPageMD5.size() << " md5 values of visited pages" << endl;
}

//LB_c: tse_ipblock文件中记录了一条条的IpBlock记录，每一条有两个地址构成(strA, strB)
// strA为网络号，strB为主机号的掩码，strB取反可以得到网络号的掩码，任意一个ip地址，
// 如ip，ip & (~strB)可以得到该ip地址的网络号，然后与strA比较，如果相同则说明ip属于
// 该网络，即属于该IpBlock。
void CCrawl::GetIpBlock()
{
	//LB_c: IP_BLOCK_FILE = "tse_ipblock"
	ifstream ifsIpBlock(IP_BLOCK_FILE.c_str());
	if (!ifsIpBlock){
		//cerr << "Cannot open " << IP_BLOCK_FILE << " for input." << endl;
		return;
	}
	string strIpBlock;
	while( getline(ifsIpBlock,strIpBlock) ){
		if(strIpBlock[0]=='\0' || strIpBlock[0]=='#' 
			|| strIpBlock[0]== '\n'){

			continue;
		}

		char buf1[64], buf2[64];
        buf1[0]='\0'; buf2[0]='\0';
        sscanf( strIpBlock.c_str(), "%s %s", buf1, buf2 );
		//LB_c: 每条IpBlock记录记录IpBlock的网络号和主机号的掩码。
		mapIpBlock.insert(valTypeIpBlock( inet_addr(buf1), inet_addr(buf2)) );	
	}
	ifsIpBlock.close();
}

void CCrawl::GetUnreachHostMD5()
{
	//vsUnreachHost.reserve(MAX_UNREACHABLE_HOST_NUM);
	//LB_c: UNREACH_HOST_FILE = "tse_unreachHost.list"
	ifstream ifsUnreachHost(UNREACH_HOST_FILE.c_str());
	if (!ifsUnreachHost){
		cerr << "Cannot open " << UNREACH_HOST_FILE << " for input." << endl;
		return;
	}
	
	string strUnreachHost;
	//int i=0;
	while( getline(ifsUnreachHost,strUnreachHost) ){
		if(strUnreachHost[0]=='\0' || strUnreachHost[0]=='#' 
			|| strUnreachHost[0]== '\n'){
			continue;
		}

		CStrFun::Str2Lower( strUnreachHost, strUnreachHost.size() );
		//vsUnreachHost.push_back(strUnreachHost);
		CMD5 iMD5;
		iMD5.GenerateMD5( (unsigned char*)strUnreachHost.c_str(), strUnreachHost.size() );
		string strDigest = iMD5.ToString();
		setUnreachHostMD5.insert(strDigest);
		//i++;
		//if(i == MAX_UNREACHABLE_HOST_NUM) break;
	}

	ifsUnreachHost.close();

}

/**************************************************************************************
 *  Function name: SaveTianwangRawData
 *  Input argv:
 *  	--	pTianwangFile: tianwang file handle
 *  	--	pUrl: url
 *  	--	pPage: web page
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save raw page data as tianwang file
**************************************************************************************/
void CCrawl::SaveTianwangRawData(CTianwangFile *pTianwangFile,
				CUrl *pUrl, CPage *pPage)
{
	if( !pTianwangFile || !pUrl || !pPage ){
		return;
	}

	file_arg arg;
	arg.pUrl = pUrl;
	arg.pPage = pPage;

	//LB_c: 将网页内容及相关数据写入天网格式文件
	// each thread writes itself, so dnnot need mutex
	pTianwangFile->Write((void*)&arg);
}

/**************************************************************************************
 *  Function name: SaveLink4SERawData
 *  Input argv:
 *  	--	pLink4SEFile: link4SE file handle
 *  	--	pUrl: url
 *  	--	pPage: web page
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save raw page data as tianwang file
**************************************************************************************/
void CCrawl::SaveLink4SERawData(CLink4SEFile *pLink4SEFile,
				CUrl *pUrl, CPage *pPage)
{
	if( !pLink4SEFile || !pUrl || !pPage ){
		return;
	}

	file_arg arg;
	arg.pUrl = pUrl;
	arg.pPage = pPage;

	// each thread writes itself, so dnnot need mutex
	pLink4SEFile->Write((void*)&arg);
}

/**************************************************************************************
 *  Function name: SaveIsamRawData
 *  Input argv:
 *  	--	pUrl: url
 *  	--	pPage: web page
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save raw page data as ISAM file
**************************************************************************************/
void CCrawl::SaveIsamRawData(CUrl *pUrl, CPage *pPage)
{
	if( !pUrl || !pPage ){
		return;
	}

	file_arg arg;
	arg.pUrl = pUrl;
	arg.pPage = pPage;

	pthread_mutex_lock(&mutexIsamFile);

	m_isamFile.Write((void *)&arg);

	pthread_mutex_unlock(&mutexIsamFile);
}

/**************************************************************************************
 *  Function name: SaveVisitedUrl
 *  Input argv:
 *  	--	url: url
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save raw the Visited Url
**************************************************************************************/
void CCrawl::SaveVisitedUrl(string url)
{
	if( m_ofsVisitedUrlFile ){
		pthread_mutex_lock(&mutexVisitedUrlFile);

		//LB_c: 已访问url存储文件，即main函数传入的visited.all文件
		m_ofsVisitedUrlFile << url << endl;

		pthread_mutex_unlock(&mutexVisitedUrlFile);
	}
}

void CCrawl::SaveUnreachHost(string host)
{
	CMD5 iMD5;
	iMD5.GenerateMD5( (unsigned char*)host.c_str(), host.size() );
	string strDigest = iMD5.ToString();
	if(  setUnreachHostMD5.find(strDigest) == setUnreachHostMD5.end() ){
		pthread_mutex_lock(&mutexUnreachHost);

		setUnreachHostMD5.insert(strDigest);
		if( m_ofsUnreachHostFile ){
			m_ofsUnreachHostFile << host << endl;
		}

		pthread_mutex_unlock(&mutexUnreachHost);
	}
}

void CCrawl::SaveLink4SE(CPage *iPage)
{
	if( m_ofsLink4SEFile && iPage->m_nRefLink4SENum>0 ){
		pthread_mutex_lock(&mutexLink4SEFile);

		m_ofsLink4SEFile << "root_url: " << iPage->m_sUrl << endl;
		m_ofsLink4SEFile << "charset: " << iPage->m_sCharset << endl;	
		m_ofsLink4SEFile << "number: " << iPage->m_nRefLink4SENum << endl;
		m_ofsLink4SEFile << "link_anchortext: " << endl;
		
		map<string,string>::iterator it4SE = iPage->m_mapLink4SE.begin();
		for( ; it4SE!= iPage->m_mapLink4SE.end(); ++it4SE ){

			m_ofsLink4SEFile << (*it4SE).first << '\t' << (*it4SE).second << endl;;

		}

		pthread_mutex_unlock(&mutexLink4SEFile);
	}
}


//LB_c: 该函数将一个页面中提取出的urls(出链)信息记录到文件link4SE.url，具体格式查看link4SE.url文件，
// 没条记录记录了page的基本信息(如version、url、ip)、网页头、和urls(出链).
// 参数arg为struct file_arg，包含了当前网页的CUrl指针和CPage指针。
bool CCrawl::SaveLink4SE031121(void  *arg)
{
	//LB_c: m_ofsLink4SEFile即link4SE.url文件
	if( !arg || !m_ofsLink4SEFile ) return false;

	//pthread_mutex_lock(&mutexLink4SEFile);

	//LB_c: vsParsedLinks就是上一部中分析页面内容，提取出来的urls(出链)
	if( vsParsedLinks.size() == 0 ) return false;

	file_arg *pFile = (file_arg *)arg;
	CUrl *iUrl = pFile->pUrl;
	CPage *iPage = pFile->pPage;

	//LB_c: 构造当前时间的字符串--------------------------------------------------------------------
	char strDownloadTime[128];
	time_t tDate;
	memset(strDownloadTime, 0, 128);
	time(&tDate);
	strftime(strDownloadTime, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	//----------------------------------------------------------------------------------------------

	//LB_c: vsParsedLinks中所有的url都写入字符串links中，每条占一行-------
	string links;
	vector<string>::iterator it = vsParsedLinks.begin();
	for( ; it!= vsParsedLinks.end(); ++it ){
		links = links + *it + "\n";
	}
	//--------------------------------------------------------------------

	//==================================== LB_c: 输出当前page的数据到文件link4SE.url ===================================
	m_ofsLink4SEFile << "version: 1.0\n";
	
	//LB_c: 输出当前page的url，如果有跳转则输出跳转url和原始url
	if( iPage->m_sLocation.size() == 0 ){
		m_ofsLink4SEFile << "url: " << iPage->m_sUrl;
	}else{
		m_ofsLink4SEFile << "url: " << iPage->m_sLocation;
		m_ofsLink4SEFile << "\norigin: " << iUrl->m_sUrl;
	}	
	//LB_c: 输出采集时间
	m_ofsLink4SEFile << "\ndate: " << strDownloadTime;	
	//LB_c: 输出host的ip
	if( mapCacheHostLookup.find(iUrl->m_sHost) == mapCacheHostLookup.end() ){
        m_ofsLink4SEFile << "\nip: " << iUrl->m_sHost;
    } else {
        m_ofsLink4SEFile << "\nip: " << ( *(mapCacheHostLookup.find(iUrl->m_sHost)) ).second;
    }
	//LB_c: 输出该page出度(即出链数)
	m_ofsLink4SEFile << "\noutdegree: " << vsParsedLinks.size();
	//LB_c: 输出从当前位置往后的数据长度
	m_ofsLink4SEFile << "\nlength: " << iPage->m_nLenHeader + links.size() + 1
		  << "\n\n" << iPage->m_sHeader << "\n";
	//LB_c: 输出urls
	m_ofsLink4SEFile << links;
	m_ofsLink4SEFile << endl;
	//==================================================================================================================

	vsParsedLinks.clear();
	//pthread_mutex_unlock(&mutexLink4SEFile);

	return true;
}

			
// not well
void CCrawl::SaveLink4History(CPage *iPage)
{
	if( m_ofsLink4HistoryFile && iPage->m_nRefLink4HistoryNum>0 ){
		pthread_mutex_lock(&mutexLink4HistoryFile);

		//m_ofsLink4HistoryFile << "root_url: " << iPage->m_sUrl << endl;
		//m_ofsLink4HistoryFile << "charset: " << iPage->m_sCharset << endl;	
		//m_ofsLink4HistoryFile << "number: " << iPage->m_nRefLink4HistoryNum << endl;
		//m_ofsLink4HistoryFile << "link: " << endl;
		
		vector<string>::iterator it4History = iPage->m_vecLink4History.begin();
		for( ; it4History!= iPage->m_vecLink4History.end(); ++it4History ){
			string s = *it4History;
			m_ofsLink4HistoryFile << s << endl;
		}

		pthread_mutex_unlock(&mutexLink4HistoryFile);
	}
}

/**************************************************************************************
 *  Function name: SaveVisitedUrlMd5
 *  Input argv:
 *  	--	md5: page md5 value
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save the visited url Md5
**************************************************************************************/
void CCrawl::SaveVisitedUrlMD5(string md5)
{
	if( m_ofsVisitedUrlMD5File ){
		m_ofsVisitedUrlMD5File << md5 << endl;
	}
}

/**************************************************************************************
 *  Function name: SaveVisitedPageMd5
 *  Input argv:
 *  	--	md5: page md5 value
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: save the visited url Md5
**************************************************************************************/
void CCrawl::SaveVisitedPageMD5(string md5)
{
	if( m_ofsVisitedPageMD5File ){
		m_ofsVisitedPageMD5File << md5 << endl;
	}
}

/**************************************************************************************
 *  Function name: OpenFileForOutput
 *  Input argv:
 *  	--
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: Open the files for output
**************************************************************************************/
void CCrawl::OpenFilesForOutput()
{
	// open isam file for output
	m_isamFile.Open(DATA_FILE_NAME, INDEX_FILE_NAME);

	// open visited.url file for output
	m_ofsVisitedUrlFile.open(m_sOutputFileName.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsVisitedUrlFile ){
		cerr << "cannot open " << VISITED_FILE << " for output\n" << endl;
	}

	// open link4SE.url file for output
	m_ofsLink4SEFile.open(LINK4SE_FILE.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsLink4SEFile ){
		cerr << "cannot open " << LINK4SE_FILE << " for output\n" << endl;
	}

	// open link4History.url file for output
	m_ofsLink4HistoryFile.open(LINK4History_FILE.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsLink4HistoryFile ){
		cerr << "cannot open " << LINK4History_FILE << " for output\n" << endl;
	}

	// open unreach host file for output
	m_ofsUnreachHostFile.open(UNREACH_HOST_FILE.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsUnreachHostFile ){
		cerr << "cannot open " << UNREACH_HOST_FILE << " for output\n" << endl;
	}

	// open visited url md5 file for output
	m_ofsVisitedUrlMD5File.open(URL_MD5_FILE.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsVisitedUrlMD5File ){
		cerr << "cannot open " << URL_MD5_FILE << " for output\n" << endl;
	}
	
	// open visited page md5 file for output
	m_ofsVisitedPageMD5File.open(PAGE_MD5_FILE.c_str(), ios::out|ios::app|ios::binary);	
	if( !m_ofsVisitedPageMD5File ){
		cerr << "cannot open " << PAGE_MD5_FILE << " for output\n" << endl;
	}
}

/***************************************************************************************
 *  Function name: DoCrawl
 *  Input argv:
 *  	--
 *  Output argv:
 *  	--
 *  Return:
 *  Function Description: the main function for crawl
 *  Be careful:
***************************************************************************************/
void CCrawl::DoCrawl()
{
	/* set the signal function */
	signal(SIGTERM, SigTerm);
	signal(SIGKILL, SigTerm);
	signal(SIGINT, SigTerm);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	// output the begin time
	char strTime[128];
	time_t tDate;

	memset(strTime,0,128);
	time(&tDate);
	strftime(strTime, 128,"%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	cout << "\n\nBegin at: " << strTime << "\n\n";

	//--- LB_c: 从外部文件中加载数据到内存中 ---
	// get the other info from file
	GetVisitedUrlMD5();
	GetVisitedPageMD5();
	GetIpBlock();
	GetUnreachHostMD5();
	//------------------------------------------

	//LB_c: 打开种子文件(即初始url文件)
	// open the seed url file
	ifstream ifsSeed(m_sInputFileName.c_str());
	if (!ifsSeed){
		cerr << "Cannot open " << m_sInputFileName << " for input\n";
		return;
	}

	//LB_c: 打开结果保存文件(已访问url，不可访问的host等)
	// open the files for output
	OpenFilesForOutput();

	//====================================  LB_c: 创建抓取线程start ====================================
	// Create thread ID structures. 
	pthread_t *tids = (pthread_t*)malloc(NUM_WORKERS * sizeof(pthread_t)); 
	if( tids == NULL){
		cerr << "malloc error" << endl;
	}
	for(unsigned int i=0; i< NUM_WORKERS; i++){
		if( pthread_create( &tids[i], NULL, start, this))
			cerr << "create threads error" << endl;
	}
	//==================================================================================================
	

	//==================================================================================================
	//LB_c: 从种子文件(即初始url文件)中取出url，调用AddUrl函数添加到待访问url集合mmapUrls中。
	string strUrl;
	CPage iCPage;
	while( getline(ifsSeed, strUrl) ){
		string::size_type idx;
		
		if(strUrl[0]=='\0' || strUrl[0]=='#' || strUrl[0]== '\n'){
			continue;
		}

		idx = strUrl.find('\t');
		if(idx != string::npos){
			strUrl = strUrl.substr(0,idx);
		}

		//idx = strUrl.find("http");
		idx = CStrFun::FindCase(strUrl, "http");
		if(idx == string::npos){
			//continue;
			idx = strUrl.find('/');
			if( idx == string::npos ){
				strUrl = "http://" + strUrl + "/";
			}else{
				strUrl = "http://" + strUrl;
			}
		}

		//if( strUrl.length() < 8 ) continue;

		if( iCPage.IsFilterLink(strUrl) ) continue;
		AddUrl(strUrl.c_str());
	}
	//==================================================================================================

	
	//==================================================================================================
	//LB_c: 从未访问url文件(即tse_unvisited.url)中取出url，调用AddUrl函数添加到待访问url集合mmapUrls中。
	// Get the unvisited URL
	ifstream ifsUnvisitedUrl(UNVISITED_FILE.c_str());
	if( ifsUnvisitedUrl ){
		while( getline(ifsUnvisitedUrl, strUrl) ){
			string::size_type idx;

			if( strUrl[0]=='\0' || strUrl[0]=='#' || strUrl[0]== '\n'){
				continue;
			}

			idx =  strUrl.find('\t');
			if(idx != string::npos){
				strUrl = strUrl.substr(0,idx);
			}

			// filter invalid urls
			if( iCPage.IsFilterLink(strUrl) ) continue;

			AddUrl(strUrl.c_str());
		}
	}else{
		//cerr << "Cannot open " << UNVISITED_FILE << " for input\n";
	}
	//==================================================================================================


	//========= LB_c: 修改工作状态标志b_fOver，等待抓取线程结束 ==========
	// sleep(30);
	b_fOver = true;
	cout << "finished to get all unvisited urls." << endl;

	// Wait for the threads. 
	for (unsigned int i = 0; i < NUM_WORKERS; ++i){
		(void)pthread_join(tids[i], NULL);
	}	
	//====================================================================
	

	cout << "closed " << NUM_WORKERS << " threads." << endl;


	//====================================================================
	//LB_c: 待访问url集合mmapUrls中剩下的urls没有访问完，将
	//其写入未访问url文件(即tse_unvisited.url)中
	SaveUnvisitedUrl();
	//====================================================================
	

	//====================================================================
	//LB_c: replicas中保存的是重复网页信息，将这些信息写入
	// 文件repli中
	SaveReplicas("repli");
	//====================================================================

	memset(strTime,0,128);
	time(&tDate);
	strftime(strTime, 128,"%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	cout << "\n\nEnd at: " << strTime << "\n\n";
}


//LB_c: 对url进行处理，然后判断是否合法、是否是境外域名、是否是不可达主机、是否已经添加等
// 其他情况则将url添加到待访问url的集合mmapUrls，其MD5值加入setUnvisitedUrlMD5。
/*****************************************************************
** Function name: AddUrl
** Input argv:
**      --
** Output argv:
**      --
** Return:
** Function Description: Add a parsed url into the collection
** Version: 1.0
** Be careful:   An important function!!!
*****************************************************************/
void CCrawl::AddUrl(const char * url)
{
	string strUrl = url;
	if( strUrl.empty() || strUrl.size() < 8 ){ //invalid url
		cout << "!so small!" << strUrl << endl;
		return;
	}
	
	CPage iCPage;
	//------------------------ LB_c: url标准化 ------------------------
    if( iCPage.NormalizeUrl(strUrl) == false ){
		cout << "!normalize fail!" << strUrl << endl;
		return;
	}
	//-----------------------------------------------------------------

	CUrl iUrl;

	//-------------------------------------------------------------------------
	//LB_c: 如果该url是图片链接，做特殊处理。如果是常规SE遇到图片链接直接返回
	//不进行后续处理，如果是ImgSE需要注释这一段。
	// for ImgSE, comment the paragraph
	// if image/xxx url, store it to link4History.url
	// begin
	if (iUrl.IsImageUrl(strUrl))
	{
		if( m_ofsLink4HistoryFile ){
			pthread_mutex_lock(&mutexLink4HistoryFile);
			m_ofsLink4HistoryFile << strUrl << endl;;
			pthread_mutex_unlock(&mutexLink4HistoryFile);
		}
		return;
	}
	// end
	//-------------------------------------------------------------------------

	//------------------- LB_c: 解析url各字段，存入iUrl对象的成员中 -----------------
	if( iUrl.ParseUrlEx(strUrl) == false ){
		cout << "ParseUrlEx error in AddUrl(): " << strUrl << endl;
		return;
	}
	//-------------------------------------------------------------------------------

	//--------------------------- LB_c: 重新检查一遍url是否有效 ---------------------
	// if it is an invalid host, discard it
	if( iUrl.IsValidHost( iUrl.m_sHost.c_str() ) == false ){
		cout << "!invalid host: " << iUrl.m_sHost << endl;    
		return;
	}
	//-------------------------------------------------------------------------------

	//---------------- LB_c: 检查url是否是foreign host,不处理foreign host -----------
	// filter foreign hosts
	if( iUrl.IsForeignHost(iUrl.m_sHost) ){
		cout << "!foreign hosts: " << iUrl.m_sHost << endl;
		return;
	}
	//-------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------
	//LB_c: IpBlock的处理，这里只对ip形式的url处理，域名形式的url需要解析(DNS解析)成
	// ip形式，将退后到CreatSocket()中处理，因为这一步是很费时的。
	//LB_c: 这一步主要是判断url是否在设定的IpBlock范围内，如果不在则丢弃
	
	// if it is a block ip, discard it
	// this work is left in the CreatSocket()
	// because the work of getting ip is inevitable in the CreatSocket function
	// 	and this work is expensive
	// if it is an unreach host, discard it
	// here we only deal with numbers-and-dots notations
	unsigned long inaddr = 0;
	char *ip = NULL;

	inaddr = (unsigned long)inet_addr( iUrl.m_sHost.c_str() );
	if ( inaddr != INADDR_NONE){ // host is just ip
		//pthread_mutex_lock(&mutexMemory);
		ip = new char[iUrl.m_sHost.size()+1];
		//pthread_mutex_unlock(&mutexMemory);
		memset(ip, 0, iUrl.m_sHost.size()+1);
		memcpy(ip, iUrl.m_sHost.c_str(), iUrl.m_sHost.size());

		//LB_c: IsValidIp函数判断该ip是否在设定的IpBlock中
		if( !iUrl.IsValidIp(ip) ){ // out of ip block
			//pthread_mutex_lock(&mutexMemory);
			delete [] ip; ip = NULL;
			//pthread_mutex_unlock(&mutexMemory);
			//cout << "!unreach hosts: " << iUrl.m_sHost << endl;
			return;
		}
		//pthread_mutex_lock(&mutexMemory);
		delete [] ip; ip = NULL;
		//pthread_mutex_unlock(&mutexMemory);
	}
	//-------------------------------------------------------------------------------

	//--------- LB_c: 求host的MD5值，判断是否在setUnreachHostMD5中，即判断是否是不可达的主机 -----------
	CStrFun::Str2Lower( iUrl.m_sHost, iUrl.m_sHost.size() );
	CMD5 iMD5;
	iMD5.GenerateMD5( (unsigned char*)iUrl.m_sHost.c_str(), iUrl.m_sHost.size() );
	string strDigest = iMD5.ToString();
	if( setUnreachHostMD5.find(strDigest) != setUnreachHostMD5.end() ){
		//cout << "!unreach host! " << iUrl.m_sHost << endl;    
		return;
	}
	//--------------------------------------------------------------------------------------------------

	//--------- LB_c: 求strUrl的MD5值，判断是否在setVisitedUrlMD5中，即判断该url是否已经抓取过 -----------
	// if crawled, discard it
	iMD5.GenerateMD5( (unsigned char*)strUrl.c_str(), strUrl.size() );
	strDigest = iMD5.ToString();
	if( setVisitedUrlMD5.find(strDigest) != setVisitedUrlMD5.end() ) {
		// cout << "!visited! " << strUrl << endl;    
		return;
	}
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------
	//LB_c: setUnvisitedUrlMD5存储未访问url的MD5值，mmapUrls存储未访问的url的字符串，这是因为进行url比较时
	// 直接比较MD5值更快。
	//LB_c: 判断该url是否已经添加，如果是否则将其MD5值strDigest计入setUnvisitedUrlMD5，
	// 后面再将strUrl加入mmapUrls。
	// if already in the collection, discard it
	if( setUnvisitedUrlMD5.find(strDigest) != setUnvisitedUrlMD5.end() ){
		// cout << "!in collection! " << strUrl << endl;    
		return;
	} else {
		pthread_mutex_lock(&mutexUnvisitedUrlMD5);
		setUnvisitedUrlMD5.insert(strDigest);
       	pthread_mutex_unlock(&mutexUnvisitedUrlMD5);
	}
	//----------------------------------------------------------------------------------------------------------
	

	// add
	// make sure limited threads crawling on a site
	int cnt = 0;
	for(;;){
		//if( mmapUrls.count(iUrl.m_sHost) < NUM_WORKERS_ON_A_SITE ){

		if(1) {
	        	//pthread_mutex_lock(&mutexVisitedUrlMD5);

			// if crawled, discard it :) double secure
			//if( setVisitedUrlMD5.find(strDigest) != setVisitedUrlMD5.end() ) {
				//cout << "!v! " << strUrl << endl;    
        			//pthread_mutex_unlock(&mutexVisitedUrlMD5);
				//return;
			//} else {

	        	pthread_mutex_lock(&mutexVisitedUrlMD5);
				//LB_c: 将该url字符串加入到待访问的集合mmapUrls
				mmapUrls.insert(mvalType( iUrl.m_sHost, strUrl));	
        		pthread_mutex_unlock(&mutexVisitedUrlMD5);
      			break;
			//}
		} else {
			cnt++;
			if( cnt % 100 == 0){
				cout << "~";
				//cnt = 0;
			}

	        // If we have waiting so long, we may remove it
            if(cnt == 50000) {
				cout << "romove it!!!!!!!!!!!!!!!!!!!" << endl;
				break;
			}
			usleep(4000);
		}

	}

}
