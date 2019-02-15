#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

using namespace std;

void Str2Lower(string& str, int nLen);

int main(int argc, char* argv[])
{
	const char *home_host[] ={
                "cn","com","net","org","info",
                "biz","tv","cc"
                //"hk","tw"
        };
	int home_host_num = 8;


	const char *f_sContentName = "tse_unreachHost.list";
	ifstream ifsContentFile(f_sContentName);
	if (!ifsContentFile ){
		cout << "Cannot open " << f_sContentName << " for input" << endl;
		exit(-1);
	}

	string line;
	while (getline(ifsContentFile,line)){
		string::size_type idx = line.rfind('.');
		string tmp;
		if( idx != string::npos ){
			tmp = line.substr(idx+1);
		}
		Str2Lower( tmp, tmp.size() );
		for(int i=0; i<home_host_num; i++){
                	if( tmp == home_host[i] )
                        	cout << line << endl;;
		}
        }

	exit(0);


}

void Str2Lower(string& str, int nLen)
{
        char distance='A'-'a' ;
                                                                                
        for( int i=0 ; i<nLen; i++ )
                if  ( str[i]>='A' && str[i]<='Z' )
                        str[i] -= distance ;
}
