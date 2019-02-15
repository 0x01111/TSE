#ifndef _COMM_H_040708_
#define _COMM_H_040708_

#include<stdlib.h>

#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>


using namespace std;

const unsigned HEADER_BUF_SIZE = 1024;
const unsigned RstPerPage = 20;

//const string IMG_INFO_NAME("./Data/s1.1");
//LB_c: 倒排索引文件（keyword->docid(s)的映射）
const string INF_INFO_NAME("./Data/sun.iidx");
//LB_c: 网页索引文件（docid->offset的映射）
const string DOC_IDX_NAME("./Data/Doc.idx");
//LB_c: 原始的天网格式网页数据
const string RAWPAGE_FILE_NAME("./Data/Tianwang.raw.2559638448");

//const string RM_THUMBNAIL_FILES("rm -f ~/public_html/ImgSE/timg/*");

//const string THUMBNAIL_DIR("/ImgSE/timg/");


#endif 
