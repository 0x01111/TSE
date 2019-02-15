// ./CrtInvertedIdx moon.fidx.sort > sun.iidx
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[])
{
	ifstream ifsImgInfo(argv[1]);
	if (!ifsImgInfo) {
		cerr << "Cannot open " << argv[1] << " for input\n";
		return -1;
	}

	string strLine,strDocNum,tmp1="";
	int cnt = 0;
	while (getline(ifsImgInfo, strLine)) {
		string::size_type idx;
		string tmp;


		idx = strLine.find("\t");
		tmp = strLine.substr(0,idx);

		if (tmp.size()<2 || tmp.size() > 8) continue;

		if (tmp1.empty()) tmp1=tmp;

		if (tmp == tmp1) {
			strDocNum = strDocNum + " " + strLine.substr(idx+1);
		} else {
			if ( strDocNum.empty() )
			{
				cerr << "------- enter if!\n";
				strDocNum = strDocNum + " " + strLine.substr(idx+1);
			}

			cout << tmp1 << "\t" << strDocNum << endl;
			tmp1 = tmp;
			strDocNum.clear();
			strDocNum = strDocNum + " " + strLine.substr(idx+1);
		}

		cnt++;
		//if (cnt==100) break;
	}
	cout << tmp1 << "\t" << strDocNum << endl;

	return 0;
}
