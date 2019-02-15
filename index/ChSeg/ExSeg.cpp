// demonstart using HzSeg
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "Dict.h"
#include "HzSeg.h"

using namespace std;

CDict iDict;

int main(int argc, char* argv[])
{
	if (argc != 2) { 
		cout << "Usage: HzstrSeg filename" << endl; 
		exit(0); 
	} 

	string FileName = argv[1];

	ifstream fin(FileName.c_str());
	ofstream fout((FileName + ".seg").c_str());

	string line;
	CHzSeg iHzSeg;  // segment instance

	while(getline(fin, line)) { 
		line = iHzSeg.SegmentSentenceMM(iDict,line); 
		fout << line << endl; 
	} 
	fin.close(); 
	fout.flush(); 
	fout.close();

	return(0);
}
