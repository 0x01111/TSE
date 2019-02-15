#include "Tse.h"
#include "Search.h"
#include "Url.h"

CSearch::CSearch()
{
}


CSearch::~CSearch()
{
}

void CSearch::DoSearch()
{
	for(;;){

		cout << "*********************************************" << endl;
		cout << "Tse searching mode." << endl;
		cout <<	"*********************************************" << endl;
		cout << "a.Search a key word" << endl;
		cout << "b.Search a URL" << endl;
		cout << "c.Exit" << endl;
		cout << "*********************************************" << endl;

		string c;
		for(;;){
			cin >> c;
			if( c == "a" || c == "b" || c == "c" ) break;
		}

		if( c == "a"){
			cout << "Please enter the query words:" << endl;

			string s;
			cin >> s;

			int num = FindKey(s.c_str());
			cout << "There are " << num << " pages containing the key,"
				<< " see the above information." << endl << endl;

			continue;

		}

		if( c == "b"){
			cout << "Please enter the url:" << endl;

			string s;
			cin >> s;

			char *sContentBuf=NULL;
			if( FindUrl(s.c_str(), &sContentBuf) == -1 ){
                        	printf("Haven't found the url.\n");
			}else{
				//cout << content << endl;

				for(unsigned int i=512; (i<strlen(sContentBuf))&&(i<600); i++){
					if((sContentBuf[i] == '\n') || (sContentBuf[i] == '\r')){
						sContentBuf[i] = '\0';
						break;
					}
					if(i == 599){
						sContentBuf[i] = '\0';
					}
				}

				cout << endl << sContentBuf << endl << "......" << endl << endl;

				if (sContentBuf)
				{
					free(sContentBuf); sContentBuf=NULL;
				}
			}

			cout << "b over" << endl;
			continue;
			
                }


		if( c == "c" ) exit(0);
	}
}




/*
 * Find results pages containing query words.
 * Return the number of results.
 */
int CSearch::FindKey(const char* key)
{
	FILE *fpfile;
	int flag=2,fpos,c,count=0;
	char fbuffer[URL_LEN],buffer[URL_LEN],url[URL_LEN];

	fpfile = fopen(DATA_FILE_NAME.c_str(),"r");
	if(fpfile == NULL){
		return -1;
	}	
	
	while( (c=fgetc(fpfile)) != EOF ){
		int b;

		if(c=='<'){
			flag=1;
		}
		if(c=='>'){
			flag=2;
		}

		// for Chinese words
		b=key[0]<0 ? 256+key[0]: key[0];

		if(b!=c) continue;

		fpos = ftell(fpfile);
		memset(fbuffer, 0, URL_LEN);

		size_t result;
        result = fread(fbuffer,1,strlen(key)-1,fpfile);
		if( result!=strlen(key)-1 && memcmp(key+1,fbuffer,strlen(key)-1) !=0 ){
			fseek(fpfile,fpos,SEEK_SET);
			continue;
		}

		if(flag==1) continue;		// <,in <>

		fseek(fpfile,-25,SEEK_CUR);
		memset(buffer, 0, URL_LEN);
		result = fread(buffer,1,50,fpfile);
        if (result != 50) continue;
		fseek(fpfile,-25,SEEK_CUR);

		//make sure not to meet the end of file here
		for(;;){
			if(feof(fpfile)) return -1;

			if(fgetc(fpfile)!=1) continue;

			if(feof(fpfile)) return -1;

			if(fgetc(fpfile)!=1) continue;

			if(feof(fpfile)) return -1;

			if(fgetc(fpfile)!=1) continue;

			if(feof(fpfile)) return -1;

			if(fgetc(fpfile)!=91) continue;

			int i=0;
			int charVal;
			char* charPos;

			memset(url, 0, URL_LEN);
			while( (charVal = fgetc(fpfile)) ){
				if(charVal==93) break;

				url[i]=charVal;
				i++;
			}
			url[i]=0;
			printf("###[%s] contains the key.\n",url);
			printf("......");

			i=0;
			while(buffer[i]==0) i++;

			charPos = buffer+i;
			charPos[49] = '\0';
			printf("%s",charPos);
			printf("......\n");

			count++;
			break;
		}
		cout << "What to do next?(c:continue/q:quit/s:"
			<< "show the correspanding web page)." << endl;
		for(;;){
			string c;
			for(;;){
				cin >> c;
				if( c == "c" || c == "q" || c == "s" ) break;
			}
			
			if( c == "c" ) break;
			if( c == "q" ) {
				fclose(fpfile);
				return count;
			}
			if( c == "s" ){
				char *sContentBuf=NULL;

				if( FindUrl(url, &sContentBuf) == -1 ){
					printf("Haven't found the url.\n");
				}else{
					for(unsigned int i=512; (i<strlen(sContentBuf))&&(i<600); i++){
						if((sContentBuf[i] == '\n') || (sContentBuf[i] == '\r')){
							sContentBuf[i] = '\0';
							break;
						}
						if(i == 599){
							sContentBuf[i] = '\0';
						}
					}

					cout << endl << sContentBuf << endl << "......" << endl << endl;

					if (sContentBuf)
					{
						free(sContentBuf); sContentBuf=NULL;
					}
				}

				cout << "What to do next?(c:continue/q:quit/s:"
					<< "show the correspanding web page)." << endl;

			}	
		}
	}

	fclose(fpfile);
	return count;
}


/*
 * If content is NULL, find out wheather INDEX_FILE includes the URL.
 * Otherwise, read the corresponding page of the URL.
 *
 */
int CSearch::FindUrl(const char *url, char **content)
{
	FILE *fpfile,*fpindex;
	char *buffer,file[URL_LEN+1];
	int offsett, offset2;

	if(content){
		fpfile = fopen(DATA_FILE_NAME.c_str(),"r");
		if(fpfile == NULL) return -1;
	}

	fpindex = fopen(INDEX_FILE_NAME.c_str(),"r");
	if(fpindex == NULL) return -1;

	while(!feof(fpindex)){
		fscanf(fpindex, "%d %s", &offsett, file);

		//cout << "offset: " << offsett << endl;
		//cout << "url: " << file << endl;
		
		if( strlen(url) > strlen(file) ){
			if( strncmp(url, file, strlen(url)) ){
				continue;
			}
		}else{
			if( strncmp(url, file, strlen(file)) ){
				continue;
			}
		}
		
		if(content == NULL){
			fclose(fpindex);
			return 0;
		}

		fseek( fpfile, offsett, 0 );
		fscanf( fpindex, "%d", &offset2 );
		offset2 = offset2 - offsett;

		buffer = ( char* )malloc( offset2+1 );
		if(buffer == NULL){
			fclose(fpfile);
			fclose(fpindex);
			return -1;
		}
		memset(buffer,0,offset2+1);
        size_t result;
		result = fread(buffer, 1, offset2, fpfile);
        if (result != offset2) continue;

		if (content == NULL)
		{
			free(buffer); buffer = NULL;
		}else
			*content = buffer;

		if(content) fclose(fpfile);
		fclose(fpindex);

		return 0;
	}
	
	if(content) fclose(fpfile);
	fclose(fpindex);

	return -1;
}
