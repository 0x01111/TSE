#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <list>

#include "Comm.h"
#include "Query.h"
#include "Document.h"
#include "StrFun.h"
#include "ChSeg/Dict.h"
#include "ChSeg/HzSeg.h"
#include "DisplayRst.h"

using namespace std;

/*
 * A inverted file(INF) includes a term-index file & a inverted-lists file.
 * A inverted-lists consists of many bucks(posting lists).
 * The term-index file is stored at vecTerm, and
 * the inverted-lists is sored at mapBuckets.
 */
int main(int argc, char* argv[])
{
	struct timeval begin_tv, end_tv;
	struct timezone tz;

	static int nCnt = 0;
	++nCnt;

	//================================ LB_c: 准备数据 ================================ 
	//LB_c: 字典对象，定义在./ChSeg/Dict.h中，该类的构造函数中会加载字典字典文件
	// (words.dict)。 
	CDict iDict;

	//LB_c: mapBuckets为倒排索引表，保存keyword->docid的映射
	map<string, string> dictMap, mapBuckets;

	//LB_c: vecDocIdx存储网页索引表，DocIdx类定义在Document.h中，是docid到offset的映射
	//关系.
	vector<DocIdx> vecDocIdx;
	//================================ LB_c: 准备数据 ================================ 

	
	//============================== LB_c: 获取用户输入 ============================== 
	//LB_c: 该类为搜索功能的实现类，提供加载数据文件、获取用户输入、中文分词、检索关
	// 键词等接口。
	CQuery iQuery;

	//LB_c: 获取用户输入的搜索字符串，利用CGI程序读取环境变量的方法。
	iQuery.GetInputs();
	
	// current query & result page number
	//LB_c: 设置搜索字符串到成员变量m_sQuery
	iQuery.SetQuery();	

	//LB_c: 设置显示查询结果的第几页（因为搜索结果可能有很多页，但是一次只能显示一页）
	// 用户提交的URL中有一个名为start的键值对指示显示第几页，如用户在结果页面中手动选
	// 择显示第几页时，或者在结果页面中提交新查询时。但是用户从首页提交搜索时URL中没有
	// start键值，所以这里需要设置一个默认值，这里的SetStart函数有一点点问题，我对其进
	// 行了修改。
	iQuery.SetStart();
	//============================== LB_c: 获取用户输入 ============================== 

	
	// begin to search
	gettimeofday(&begin_tv,&tz); 


	//================================ LB_c: 准备数据 ================================ 
	//LB_c: 读取倒排索引文件(./Data/sun.iidx)，将keyword->docid的映射存入mapBuckets中
	iQuery.GetInvLists(mapBuckets); 

	//LB_c: 读取网页索引文件(./Data/Doc.idx)，将docid->offset的映射存入vecDocIdx中
	iQuery.GetDocIdx(vecDocIdx); 
	//================================ LB_c: 准备数据 ================================ 


	//================================ LB_c: 中文分词 ================================ 
	//LB_c: 中文分词类，定义于HzSeg.h 
	CHzSeg iHzSeg;	
	
	//LB_c: 该函数进行中文分词，iDict字典对象。将原字符串中插入"/"进行分割，分割后的
	// 字符串存入m_sSegQuery.
	iQuery.m_sSegQuery = iHzSeg.SegmentSentenceMM(iDict,iQuery.m_sQuery); 
	string strSegRes = iQuery.m_sSegQuery;
	
	//LB_c: 存放分割后的独立的关键词
	vector<string> vecTerm; 
	//LB_c: 从分割后的串m_sSegQuery中获取独立的关键词一一存入vector中
	iQuery.ParseQuery(vecTerm); 
	//================================ LB_c: 中文分词 ================================ 
	

	//=========================== LB_c: 检索关键词+结果排序 ========================== 
	//LB_c: 存储搜索结果的网页的docid
	set<string> setRelevantRst; 
	
	//LB_c: vecTerm中存储的分割后的独立关键词，mapBuckets存储的倒排表，在倒排表中分别
	// 检索verTerm中的所有独立关键词，然后进行简单的结果排序。实际上结果排序非常重要，
	// 要达到很好的效果算法也非常复杂，TSE系统中只是简单的实现了结果排序，所以与关键词
	// 检索在一个函数里实现。
	iQuery.GetRelevantRst(vecTerm, mapBuckets, setRelevantRst); 
	//=========================== LB_c: 检索关键词+结果排序 ========================== 
	

	gettimeofday(&end_tv,&tz);
	// search end


	//============================== LB_C: 显示搜索结果 ============================== 
	//LB_c: 结果显示类，类中主要有ShowTop,ShowMiddle和ShowBelow三个接口，用标准输出
	// html语句的方式显示结果页面。
	CDisplayRst iDisplayRst; 
	
	//LB_c: 显示结果页面的头部
	iDisplayRst.ShowTop(); 

	//LB_c: 计算搜索过程耗费的时间
	float used_msec = (end_tv.tv_sec-begin_tv.tv_sec)*1000 
		+((float)(end_tv.tv_usec-begin_tv.tv_usec))/(float)1000; 
	
	//LB_c: 显示结果页面的中部，主要是一些结果信息：用户的查询串、耗时、找到的结果页面
	// 总数和分页显示的分页链接等。
	iDisplayRst.ShowMiddle(iQuery.m_sQuery,used_msec, 
			setRelevantRst.size(), iQuery.m_iStart);

	//LB_c: 显示结果页面的底部，就是搜索的结果，以列表的形式列出。主要包括网页的URL，
	// 网页大小，网页内容的摘要和网页快照的链接等。
	//cout << "<p>iQuery.m_iStart : " << iQuery.m_iStart << "</p>\n";
	//cout << "<p>iQuery.m_sSegQuery : " << strSegRes << "</p>\n";
	iDisplayRst.ShowBelow(vecTerm,setRelevantRst,vecDocIdx, iQuery.m_iStart); 
	//============================== LB_C: 显示搜索结果 ============================== 


	return 0;

}
