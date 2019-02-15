/******************************************************************
 ** File name:   CgIsamFile.cpp
 ** Copyright (c):
 ** Function Description: implementation of the CgIsamFile class.
 ** Author:
 ** Created time:
 ** Modified history:
 ** Version:
 ** Be careful:
 ******************************************************************/

#include "IsamFile.h"
#include "Url.h"
#include "Page.h"

///////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////
CIsamFile::CIsamFile()
{
	fpDataFile = NULL;
	fpIdxFile = NULL;
}

CIsamFile::CIsamFile(string str) : CFileEngine(str)
{
	fpDataFile = NULL;
	fpIdxFile = NULL;
}

CIsamFile::CIsamFile(string strData, string strIndex)
{
	fpDataFile = NULL;
	fpIdxFile = NULL;

	m_str = strData;
	m_sIndexFileName = strIndex;

	fpDataFile = fopen(DATA_FILE_NAME.c_str(), "a");
	if( fpDataFile == NULL ){
		return;
	}

	fpIdxFile = fopen(INDEX_FILE_NAME.c_str(), "a");
	if( fpIdxFile == NULL ){
		return;
	}
}

CIsamFile::~CIsamFile()
{
}

bool CIsamFile::Open(string strData, string strIndex)
{
	m_str = strData;
	m_sIndexFileName = strIndex;

	fpDataFile = fopen(DATA_FILE_NAME.c_str(), "a");
	if( fpDataFile == NULL ){
		return false;
	}

	fpIdxFile = fopen(INDEX_FILE_NAME.c_str(), "a");
	if( fpIdxFile == NULL ){
		return false;
	}

	return true;
}

void CIsamFile::Close()
{
	fclose(fpIdxFile);
	fclose(fpDataFile);
}


/************************************************************************
 *  Function name: Write
 *  Input argv:
 *  	-- arg: the file_arg handle contain the url & page data
 *  Output argv:
 *  	--
 *  Return:
 *  	true: success
 *  	false: fail
************************************************************************/
bool CIsamFile::Write(void *arg)
{
	if( !arg || !fpIdxFile || !fpDataFile ){
		return false;
	}

	file_arg *pFile = (file_arg *)arg;

	CUrl *iUrl = pFile->pUrl;
	CPage *iPage = pFile->pPage;

	const char* url = NULL;
	const char* buffer = NULL;

	if( iPage->m_sLocation.length() == 0 ){
		url = iUrl->m_sUrl.c_str();
	} else {
		url = iPage->m_sLocation.c_str();
	}

	buffer = iPage->m_sContent.c_str();
	int len = iPage->m_sContent.length();
	/////////////////////////////////////////////

	int offsett = ftell(fpDataFile);
	fprintf(fpIdxFile, "%10d", offsett);
	fprintf(fpIdxFile, "%256s\n", url);
	fflush(fpIdxFile);

	fwrite( buffer, 1, len, fpDataFile); 

	//write 25 spaces in the file
	for(int i=0; i<25; i++){
		fputc(0,fpDataFile);
	}

	//write 3 '1' in the file
	fputc(1,fpDataFile);
	fputc(1,fpDataFile);
	fputc(1,fpDataFile);

	//write [url] in the file
	fputc(91,fpDataFile);
	fwrite( url, 1, strlen(url), fpDataFile);
	fputc(93,fpDataFile);

	for(int i=0; i<25; i++){
		fputc(0,fpDataFile);
	}

	fflush(fpDataFile);

	return true;
}
