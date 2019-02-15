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

//LB_c: �����ظ���ҳʱ�ã�firstΪpage���ݵ�MD5ֵ��secondΪpage��Ӧ��url�ַ���
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

//LB_c: �����ݵĸ�ʽ˵���� CCrawl::GetIpBlock()
map<unsigned long,unsigned long> mapIpBlock;
//LB_c: ���ץȡ���̵�״̬��ץȡ�߳�ͨ���жϸñ�־������ץȡ
bool b_fOver;
//LB_c: �洢���ڴ��еĴ�ץȡurl��¼��ÿ����¼��һ��Ϊurl��host(������)���ڶ���Ϊurl�ַ���
multimap<string,string > mmapUrls;

typedef map<unsigned long,unsigned long>::value_type valTypeIpBlock;
typedef map<string,string>::value_type mvalType;

void SaveReplicas(const char* filename);

struct package
{
	CCrawl *crawl;
	CPage *page;
};

//LB_c: �洢��ǰҳ������ȡ����url
vector<string> vsParsedLinks;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LB_c: onfind��һ���ص�������hlink_detect_string������ÿ�ҵ�һ��url����øûص�������
// ��url���뵽�����ʵļ���mmapUrls��������vsParsedLinks�С�����uri������ȡ����url��Ϣ������elemӦ��
// ��ʾ��url�����ͣ����ǲ���ͼƬ���ӣ��ȼ���CUrl���IsImageUrl�����Ĺ��ܡ�
int onfind(const char *elem, const char *attr, struct uri *uri, void *arg)
{
	struct package *p=(struct package*)arg;
	char buff[URL_LEN+1];
	
	//LB_c: �ú����ǽ�struct uri��ʽ��url��Ϣ������װ���ַ�����ʽ���洢��buff
	if (uri_recombine(uri, buff, URL_LEN+1, C_SCHEME| C_AUTHORITY| C_PATH| C_QUERY ) >= 0)
	{
		//LB_c: ���ַ�����ʽ��url(buff)����vsParsedLinks
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
				//LB_c: ����ܹؼ��������ȡ����url��һ����Ч�����ӣ���Ҫ���뵽��ץȡ����mmapUrls�У�
				// ץȡ�̲߳��ϴӸü�����ȡ��url����ץȡ��
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
	//LB_c: nGSock�Ǵ�������sock����¼��һ�ε�socket���ӣ�nSock����ʱ��
	int nSock = nGSock;

	cout << "1. pid=" << pthread_self() << " sock = " << nGSock << endl;

	//LB_c: CHttp��httpЭ��Ĺؼ�������
	CHttp http;
	//LB_c: Fetch��CHttp��Ĺؼ���Ҫ�ӿڣ��ú�������url����http���Ӳ�������ҳ
	file_length = http.Fetch(iUrl.m_sUrl, &downloaded_file, &fileHead, &location, &nSock);

#ifdef DEBUG	// just download
	cout << "######file length: ######" << file_length << endl;
	cout << "######head: ######" << fileHead << endl;
#endif

	//========================================= LB_c: http.Fetch����-300������URL��ת =======================================
	int nCount = 0;
	while( file_length == -300 ){ // moved to an another place
		//LB_c: �����ת�ĵ�ַlocation���ȷǷ�������������ת3�Σ���ֹͣ����
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
		//LB_c: idx1!=0˵��strUrlLocation��ͷ����"http"������http˵����ͬһ��host�ڲ���ת��
		// ����iUrl��strUrlLocation�������ת��url�� idx == 0˵��strUrlLocationͷ����"http"��
		// ˵����һ��������url������Ҫ���⴦��
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
		//LB_c: ����������ҳ
		if( iPage.IsFilterLink(strUrlLocation) ){
			file_length = -1;
			break;
		}

		cout << "2. pid=" << pthread_self() << " sock = " << nGSock << endl;
		//LB_c: ����������ת����ҳ
		file_length = http.Fetch( strUrlLocation, &downloaded_file, &fileHead, &location, &nSock);
		nCount++;
	}
	//=======================================================================================================================

	//LB_c: �����µ�����nSock��nGSock
	nGSock = nSock;

	//================================== LB_c: http.Fetch����-1,�����ͷ���ʱ�ռ䷵�� ======================================
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

	//============================= LB_c: http.Fetch����-2,out of ip block���ͷ���ʱ�ռ䷵�� ================================
	if(file_length == -2){ // out of ip block .
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}

		//LB_?: ��ǰurl����ip block�У��Ƿ�Ҫ��host����UnreachHost!? ��Ϊ����unreach���´α任��ipblock�Ƿ����Ӱ��?
		// save unreach host
        SaveUnreachHost(iUrl.m_sHost);
		cout << "-out of block host: " << iUrl.m_sHost << endl;;
		return;
	}
	//=======================================================================================================================
	
	//=========================== LB_c: http.Fetch����-3,invalid host or ip���ͷ���ʱ�ռ䷵�� ===============================
	if(file_length == -3) { // invalid host or ip
		if (fileHead)
		{
			free(fileHead); fileHead=NULL;
		}
		if (downloaded_file)
		{
			free(downloaded_file); downloaded_file=NULL;
		}
		
		//LB_?: ò��������Ҫ��host����UnreachHost����Ϊ��ǰip��host����Ч��
		
		cout << "-invalid host: " << iUrl.m_sHost << endl;
		return;
	}
	//=======================================================================================================================

	//============================ LB_c: http.Fetch����-3,��ǰurl��ͼƬ���ӣ��ͷ���ʱ�ռ䷵�� ===============================
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

	//============== LB_c: ��ҳͷ������ҳbody����Ϊ����Ϊ�쳣���ͷ���ʱ�ռ䣬���� ==============
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

	//==================================== LB_c: ����CPage����󣬽��н��� ====================================
	CPage iPage(iUrl.m_sUrl, strUrlLocation, fileHead, downloaded_file, file_length);
	if (fileHead)
	{
		free(fileHead); fileHead=NULL;
	}
	if (downloaded_file)
	{
		free(downloaded_file); downloaded_file=NULL;
	}

	//LB_c: ������ҳͷ��Ϣ��������浽CPage���Ա��
	iPage.ParseHeaderInfo(iPage.m_sHeader);

	//LB_c: �������Ƿ�֧�ֱ������ӣ�����ر�socket
	if( iPage.m_bConnectionState == false ){
		close(nGSock);
		nGSock = -1;
	}
	//=========================================================================================================
	

	//======================= LB_c: ��һ���Ǽ����ҳ�����������Ƿ���ȷ =======================
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


	//================================== LB_c: �ⲿ���ǽ�ѹ��gzip��ʽѹ������ҳ���� ==================================
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


	//======= LB_c: �ⲿ���ǽ���ǰ��url��MD5ֵ���뵽setVisitedUrlMD5����д���ļ�tse_md5.visitedurl�� =======
	CMD5 iMD5;
	string strDigest;

	/////////////////////////////
	// because we can make sure the url in the setVisitedUrlMd5
	// is not same(we have check it before insert it to the collection),
	// we intert it directly.  however...
	//iMD5.GenerateMD5( (unsigned char*)iPage.m_sUrl.c_str(), iPage.m_sUrl.length() );
	iMD5.GenerateMD5( (unsigned char*)iUrl.m_sUrl.c_str(), iUrl.m_sUrl.length() );
	strDigest = iMD5.ToString();
	
	//LB_c: �ٴμ���url��MD5ֵ�Ƿ��Ѿ��ڼ���setVisitedUrlMD5����˵���Ѿ����ʹ�������!
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
	//LB_c: ��һ�δ����ظ�����ҳ���ݣ�ע�����ظ�����ҳ���ݶ������ظ���url������ͬ��url��������ȫһ���� ���˾�������
	// ��������岻����Ϊ������ͨ����������ҳ��������MD5ֵ���ж��ظ��ģ�����������ҳֻҪ��һ���ַ���ͬ��������ҳ
	// ��MD5ֵҲ�᲻ͬ����˲����ظ���ҳ���������ش�����simhash����������!
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


	//LB_c: ����ҳ���ݼ��������д��������ʽ�ļ� ============================
	////////////////////
	// save as Tianwang format
	SaveTianwangRawData(pTianwangFile, &iUrl, &iPage);
	//=======================================================================


	//=======================================================================
	//LB_c: ��url���ʳɹ�����url��¼���ļ��У����û����ת��¼ԭʼurl����
	//��ת���¼ʵ�ʵ�ַm_sLocation��LB_?: ����˵�����ԭʼurlҲ�㱻ץȡ
	//�ˣ�Ϊʲô��Ҳ��ӵ��ļ�����?
	////////////////////
	// save visited Urls
	if( iPage.m_sLocation.length() < 1 ){
		SaveVisitedUrl(iUrl.m_sUrl);
	} else {
		SaveVisitedUrl(iPage.m_sLocation);
	}
	//=======================================================================

	//return;	// just crawl seeds


	//================================= LB_c: �����Ǵ�ҳ��(iPage)����ȡurl ===================================

	//LB_c: ֻ֧��text/html��ʽ����ҳ����ȡurl
	/////////////////////////////////////
	// Parse hyperlinks
	if (iPage.m_sContentType != "text/html") { // we can only find links in tex/html
		return;
	}
	// using XIE Han's link parser

    struct uri page_uri;

	pthread_mutex_lock(&mutexDetect);

	//--------------------------------- LB_c: url���� --------------------------------------------
	if (iPage.m_sLocation.empty())
	{
		//LB_?: �ú���û�ҵ�Դ��
		//LB_c: C����ʵ�ֵ�url������������CUrl��ͬ��<<����>>������˵������Ϊ�˼�����һ���ԡ�
		// ������1��url�ַ���������url�ṹ�壬��������url�ĸ��ֶΣ���struct uri���塣
		uri_parse_string(iPage.m_sUrl.c_str(), &page_uri);
	}
	else
	{
		uri_parse_string(iPage.m_sLocation.c_str(), &page_uri);
	}
	//--------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------
	//LB_c: ҳ���������ȡurls(����)��onfind��һ���ص�������hlink_detect_string������ÿ�ҵ�һ��
	//url����øûص���������url���뵽�����ʵļ���mmapUrls��������vsParsedLinks�д���һ��
	//SaveLink4SE031121������д���ļ��С�	
	struct package p={this, &iPage};
	hlink_detect_string(iPage.m_sContent.c_str(), &page_uri, onfind, &p);
	//--------------------------------------------------------------------------------------------

	//-----------------------------------------------------------------------
	//LB_c: ����ǰҳ������ȡ����urls(����)��Ϣ��¼���ļ�link4SE.url�� ---
	struct file_arg pLinks = {&iUrl, &iPage};
	SaveLink4SE031121( &pLinks );
	//-----------------------------------------------------------------------

	// save as Link4SE format
	//SaveLink4SERawData(pLink4SEFile, &iUrl, &iPage);

	pthread_mutex_unlock(&mutexDetect);

	//LB_c: �����page_uri�Ѿ���onfind�ͷŹ���
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

//LB_c: inputFileNameΪurl�����ļ�������ʼ�Ĵ���ȡ��ҳ��url
// outputFileNameΪ��ȡ��¼�ļ�����¼�����Ѿ���ȡ��url
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

//LB_c: tse_ipblock�ļ��м�¼��һ������IpBlock��¼��ÿһ����������ַ����(strA, strB)
// strAΪ����ţ�strBΪ�����ŵ����룬strBȡ�����Եõ�����ŵ����룬����һ��ip��ַ��
// ��ip��ip & (~strB)���Եõ���ip��ַ������ţ�Ȼ����strA�Ƚϣ������ͬ��˵��ip����
// �����磬�����ڸ�IpBlock��
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
		//LB_c: ÿ��IpBlock��¼��¼IpBlock������ź������ŵ����롣
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

	//LB_c: ����ҳ���ݼ��������д��������ʽ�ļ�
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

		//LB_c: �ѷ���url�洢�ļ�����main���������visited.all�ļ�
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


//LB_c: �ú�����һ��ҳ������ȡ����urls(����)��Ϣ��¼���ļ�link4SE.url�������ʽ�鿴link4SE.url�ļ���
// û����¼��¼��page�Ļ�����Ϣ(��version��url��ip)����ҳͷ����urls(����).
// ����argΪstruct file_arg�������˵�ǰ��ҳ��CUrlָ���CPageָ�롣
bool CCrawl::SaveLink4SE031121(void  *arg)
{
	//LB_c: m_ofsLink4SEFile��link4SE.url�ļ�
	if( !arg || !m_ofsLink4SEFile ) return false;

	//pthread_mutex_lock(&mutexLink4SEFile);

	//LB_c: vsParsedLinks������һ���з���ҳ�����ݣ���ȡ������urls(����)
	if( vsParsedLinks.size() == 0 ) return false;

	file_arg *pFile = (file_arg *)arg;
	CUrl *iUrl = pFile->pUrl;
	CPage *iPage = pFile->pPage;

	//LB_c: ���쵱ǰʱ����ַ���--------------------------------------------------------------------
	char strDownloadTime[128];
	time_t tDate;
	memset(strDownloadTime, 0, 128);
	time(&tDate);
	strftime(strDownloadTime, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	//----------------------------------------------------------------------------------------------

	//LB_c: vsParsedLinks�����е�url��д���ַ���links�У�ÿ��ռһ��-------
	string links;
	vector<string>::iterator it = vsParsedLinks.begin();
	for( ; it!= vsParsedLinks.end(); ++it ){
		links = links + *it + "\n";
	}
	//--------------------------------------------------------------------

	//==================================== LB_c: �����ǰpage�����ݵ��ļ�link4SE.url ===================================
	m_ofsLink4SEFile << "version: 1.0\n";
	
	//LB_c: �����ǰpage��url���������ת�������תurl��ԭʼurl
	if( iPage->m_sLocation.size() == 0 ){
		m_ofsLink4SEFile << "url: " << iPage->m_sUrl;
	}else{
		m_ofsLink4SEFile << "url: " << iPage->m_sLocation;
		m_ofsLink4SEFile << "\norigin: " << iUrl->m_sUrl;
	}	
	//LB_c: ����ɼ�ʱ��
	m_ofsLink4SEFile << "\ndate: " << strDownloadTime;	
	//LB_c: ���host��ip
	if( mapCacheHostLookup.find(iUrl->m_sHost) == mapCacheHostLookup.end() ){
        m_ofsLink4SEFile << "\nip: " << iUrl->m_sHost;
    } else {
        m_ofsLink4SEFile << "\nip: " << ( *(mapCacheHostLookup.find(iUrl->m_sHost)) ).second;
    }
	//LB_c: �����page����(��������)
	m_ofsLink4SEFile << "\noutdegree: " << vsParsedLinks.size();
	//LB_c: ����ӵ�ǰλ����������ݳ���
	m_ofsLink4SEFile << "\nlength: " << iPage->m_nLenHeader + links.size() + 1
		  << "\n\n" << iPage->m_sHeader << "\n";
	//LB_c: ���urls
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

	//--- LB_c: ���ⲿ�ļ��м������ݵ��ڴ��� ---
	// get the other info from file
	GetVisitedUrlMD5();
	GetVisitedPageMD5();
	GetIpBlock();
	GetUnreachHostMD5();
	//------------------------------------------

	//LB_c: �������ļ�(����ʼurl�ļ�)
	// open the seed url file
	ifstream ifsSeed(m_sInputFileName.c_str());
	if (!ifsSeed){
		cerr << "Cannot open " << m_sInputFileName << " for input\n";
		return;
	}

	//LB_c: �򿪽�������ļ�(�ѷ���url�����ɷ��ʵ�host��)
	// open the files for output
	OpenFilesForOutput();

	//====================================  LB_c: ����ץȡ�߳�start ====================================
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
	//LB_c: �������ļ�(����ʼurl�ļ�)��ȡ��url������AddUrl������ӵ�������url����mmapUrls�С�
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
	//LB_c: ��δ����url�ļ�(��tse_unvisited.url)��ȡ��url������AddUrl������ӵ�������url����mmapUrls�С�
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


	//========= LB_c: �޸Ĺ���״̬��־b_fOver���ȴ�ץȡ�߳̽��� ==========
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
	//LB_c: ������url����mmapUrls��ʣ�µ�urlsû�з����꣬��
	//��д��δ����url�ļ�(��tse_unvisited.url)��
	SaveUnvisitedUrl();
	//====================================================================
	

	//====================================================================
	//LB_c: replicas�б�������ظ���ҳ��Ϣ������Щ��Ϣд��
	// �ļ�repli��
	SaveReplicas("repli");
	//====================================================================

	memset(strTime,0,128);
	time(&tDate);
	strftime(strTime, 128,"%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	cout << "\n\nEnd at: " << strTime << "\n\n";
}


//LB_c: ��url���д���Ȼ���ж��Ƿ�Ϸ����Ƿ��Ǿ����������Ƿ��ǲ��ɴ��������Ƿ��Ѿ���ӵ�
// ���������url��ӵ�������url�ļ���mmapUrls����MD5ֵ����setUnvisitedUrlMD5��
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
	//------------------------ LB_c: url��׼�� ------------------------
    if( iCPage.NormalizeUrl(strUrl) == false ){
		cout << "!normalize fail!" << strUrl << endl;
		return;
	}
	//-----------------------------------------------------------------

	CUrl iUrl;

	//-------------------------------------------------------------------------
	//LB_c: �����url��ͼƬ���ӣ������⴦������ǳ���SE����ͼƬ����ֱ�ӷ���
	//�����к������������ImgSE��Ҫע����һ�Ρ�
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

	//------------------- LB_c: ����url���ֶΣ�����iUrl����ĳ�Ա�� -----------------
	if( iUrl.ParseUrlEx(strUrl) == false ){
		cout << "ParseUrlEx error in AddUrl(): " << strUrl << endl;
		return;
	}
	//-------------------------------------------------------------------------------

	//--------------------------- LB_c: ���¼��һ��url�Ƿ���Ч ---------------------
	// if it is an invalid host, discard it
	if( iUrl.IsValidHost( iUrl.m_sHost.c_str() ) == false ){
		cout << "!invalid host: " << iUrl.m_sHost << endl;    
		return;
	}
	//-------------------------------------------------------------------------------

	//---------------- LB_c: ���url�Ƿ���foreign host,������foreign host -----------
	// filter foreign hosts
	if( iUrl.IsForeignHost(iUrl.m_sHost) ){
		cout << "!foreign hosts: " << iUrl.m_sHost << endl;
		return;
	}
	//-------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------
	//LB_c: IpBlock�Ĵ�������ֻ��ip��ʽ��url����������ʽ��url��Ҫ����(DNS����)��
	// ip��ʽ�����˺�CreatSocket()�д�����Ϊ��һ���Ǻܷ�ʱ�ġ�
	//LB_c: ��һ����Ҫ���ж�url�Ƿ����趨��IpBlock��Χ�ڣ������������
	
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

		//LB_c: IsValidIp�����жϸ�ip�Ƿ����趨��IpBlock��
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

	//--------- LB_c: ��host��MD5ֵ���ж��Ƿ���setUnreachHostMD5�У����ж��Ƿ��ǲ��ɴ������ -----------
	CStrFun::Str2Lower( iUrl.m_sHost, iUrl.m_sHost.size() );
	CMD5 iMD5;
	iMD5.GenerateMD5( (unsigned char*)iUrl.m_sHost.c_str(), iUrl.m_sHost.size() );
	string strDigest = iMD5.ToString();
	if( setUnreachHostMD5.find(strDigest) != setUnreachHostMD5.end() ){
		//cout << "!unreach host! " << iUrl.m_sHost << endl;    
		return;
	}
	//--------------------------------------------------------------------------------------------------

	//--------- LB_c: ��strUrl��MD5ֵ���ж��Ƿ���setVisitedUrlMD5�У����жϸ�url�Ƿ��Ѿ�ץȡ�� -----------
	// if crawled, discard it
	iMD5.GenerateMD5( (unsigned char*)strUrl.c_str(), strUrl.size() );
	strDigest = iMD5.ToString();
	if( setVisitedUrlMD5.find(strDigest) != setVisitedUrlMD5.end() ) {
		// cout << "!visited! " << strUrl << endl;    
		return;
	}
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------
	//LB_c: setUnvisitedUrlMD5�洢δ����url��MD5ֵ��mmapUrls�洢δ���ʵ�url���ַ�����������Ϊ����url�Ƚ�ʱ
	// ֱ�ӱȽ�MD5ֵ���졣
	//LB_c: �жϸ�url�Ƿ��Ѿ���ӣ�����Ƿ�����MD5ֵstrDigest����setUnvisitedUrlMD5��
	// �����ٽ�strUrl����mmapUrls��
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
				//LB_c: ����url�ַ������뵽�����ʵļ���mmapUrls
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
