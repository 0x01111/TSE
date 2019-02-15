#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>
#include <cstdlib>
#include <algorithm>

using namespace std;

string srcFileName = "visited.all";

int main(int argc, char* argv[])
{
	if( argc!=2 ) {
		cout << "error argument!" << endl;
		exit(-1);
	}
	
	srcFileName = argv[1];

	ifstream ifsVisitedUrl(srcFileName.c_str(),ios::binary);
        if(!ifsVisitedUrl){
                cerr << "did not find " << srcFileName << " for iutput" << endl;
        }       
        
        string strUrl,strHost;
	vector<string> vecHost;
	string::size_type idx;
	vector<string>::iterator itResult;

        while( getline(ifsVisitedUrl,strUrl) ){
		idx = strUrl.find('/',7);
		if( idx != string::npos ){
			strHost = strUrl.substr( 7, idx-7);
		}

		idx = strHost.rfind(':');
		if( idx != string::npos ){
			strHost = strHost.substr(0, idx);
		}

		itResult = find(vecHost.begin(), vecHost.end(), strHost );
		if ( itResult == vecHost.end() ){
			vecHost.push_back( strHost );
		}
		//cout << "host: " << strHost << endl;
		//break;
		
        }
        
        ifsVisitedUrl.close();

	cout << endl << "total: " << vecHost.size() << " hosts." << endl;
	vector<string >::iterator it = vecHost.begin();
	for ( ; it != vecHost.end(); ++it ){
		cout << (*it) << endl;
	}

	return 0;
}
