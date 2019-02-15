/*
 * Main.cpp -- the main function of Tiny Search Engine
 * Created: YAN Hongfei, Network lab of Peking University. <yhf@net.pku.edu.cn>
 * Created: July 15 2003. version 0.1.1
 *		# Can crawl web pages with a process
 * Updated: Aug 20 2003. version 1.0.0 !!!!
 *		# Can crawl web pages with multithreads
 * Updated: Nov 08 2003. version 1.0.1 !!!!
 *		# more classes in the codes
 */

#include "Tse.h"
#include "Crawl.h"
#include "Search.h"

void help();


//LB_c: 
/* 该程序的执行格式为:	./Tse -c tse_seed.img 或 ./Tse -s
所以argv[1]为运行模式(-s搜索，-c搜集)，
argv[1]为-c时，argv[2]为url种子文件，即初始的待爬取网页的url
*/
int main(int argc, char* argv[])
{
	if( argc==2 && !strncmp(argv[1],"-s",2) ){	// search
		CSearch iSearch;
		iSearch.DoSearch();
	} else if( argc==3 && !strncmp(argv[1],"-c",2) ){ // crawl pages
		//LB_c: argv[2]为url种子文件，即初始的待爬取网页的url
		// visited.all为爬取记录文件，记录所有已经爬取的url
		CCrawl iCrawl(argv[2], "visited.all");
		iCrawl.DoCrawl();
	} else {	// invalid argv
		help();
	}

	exit(0);
}

void help()
{
	cout << "Tse 1.0" << endl;
	cout << "Tiny Web Searching Engine, July 2003" << endl;
	cout << "Syntax:" << endl;
	cout << "\tTse -c inputFileName" << endl;
	cout << "\tTse -s" << endl;
	cout << "The input file contains a list of URLs for crawling.-c is to crawl all urls of inutFileName, and to save crawled links in 'visited.all' file. -s is a searching mode.In the extended mode,all the hypertext links in the webpages will be stored in the outputfile in the same format as the input file." << endl;

	exit(0);
}
