///////////////////////////////////////////////////////////////////////////
// LB_c: 倒排文件索引（或称：反向索引）

//./ForwardDocIdx Tianwang.raw.2559638448.seg > moon.fdx
#include <iostream>
#include <fstream>

using namespace std;

const string SEPARATOR("/  ");          // delimiter between words

int main(int argc, char* argv[])
{
	ifstream ifsImgInfo(argv[1]);
	if (!ifsImgInfo) {
		cerr << "Cannot open " << argv[1] << " for input\n";
		return -1;
	}

	string strLine,strDocNum;
	int cnt = 0;
	while (getline(ifsImgInfo, strLine)) {
		string::size_type idx;

		cnt++;
		if (cnt%2 == 1){
			strDocNum = strLine.substr(0,strLine.size());
			continue;
		}
		if (strLine[0]=='\0' || strLine[0]=='#' || strLine[0]=='\n'){
			continue;
		}

		while ( (idx = strLine.find(SEPARATOR)) != string::npos ) {
			string tmp1 = strLine.substr(0,idx);
			cout << tmp1 << "\t" << strDocNum << endl;
			strLine = strLine.substr(idx + SEPARATOR.size());
		}

		//if (cnt==100) break;
	}

	return 0;
}
