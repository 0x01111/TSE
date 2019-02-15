// HzSeg handling

#include "HzSeg.h"
#include "Dict.h"

const unsigned int MAX_WORD_LENGTH = 8;
const string SEPARATOR("/  ");		// delimiter between words

CHzSeg::CHzSeg()
{
}

CHzSeg::~CHzSeg()
{
}

//LB_c: �������ƥ�䷨�����ķִ�ʵ�֣�dict�Ƿִ�����ѯ�Ĵʵ����s1�������ַ�����
//LB_c: using Max-matching-method to segment the string s1 with dictory dict.
// Using Max Matching method to segment a character string.
string CHzSeg::SegmentHzStrMM (CDict &dict, string s1) const
{
	string s2="";				// store segment result
	while (!s1.empty()) { 
		unsigned int len=s1.size();
		//LB_c: MAX_WORD_LENGHTΪ���õ����ʳ���TSE������Ϊ8���ֽڣ���4������
		if (len>MAX_WORD_LENGTH) len=MAX_WORD_LENGTH;

		//LB_c: ��s1ͷ��ȡ���ʳ����Ӵ���Ϊ��ƥ��ʣ�������������ƥ�䷨���s1��β��ȡ��
		string w=s1.substr(0, len);// the candidate word

		//LB_c: �ڴʵ��в�ѯw�Ƿ���ڣ�CDict::IsWord�����ж��ַ����Ƿ��ڴʵ��С���һ���Ӵ�Ĵʵ��в��Ҵʵ�Ч���Ǻܹؼ�
		// �ģ�����Ӱ���ϵͳЧ�ʵ�һ������֮һ����5���н��ܹ���TSE�еĴʵ�����STL��map�ṹ�洢�ģ�Ҳ���Ǻ���������ݽ�
		// ���洢�����еķִ�ϵͳҲ��hash��洢������Ϊ����߲�ѯЧ�ʡ�
		bool isw=dict.IsWord(w);

		//LB_c: ����ڴʵ���û���ҵ���ȥ�����һ���ּ�����ѯ��ֱ��wΪһ������Ϊֹ��
		while (len>2 && isw==false) {	// if not a word
			len-=2;		// cut a word
			w=w.substr(0, len);
			isw=dict.IsWord(w);
		}

		//LB_c: ��ƥ���w�����ָ��SEPARATOR���зָ
		s2 += w + SEPARATOR;

		s1 = s1.substr(w.size());
	}

	return s2;
}


//LB_c: ���ķִ�Ԥ������������Ӣ���ַ���ASCII�ַ����������ַ������ı����ŵȣ���Ԥ�����õ��ĺ����ַ�������
// SegmentHzStrMM�����������ķִʡ�����dictΪ�ʵ����s1Ϊ��������ַ�����
// process a sentence before segmentation
string CHzSeg::SegmentSentenceMM (CDict &dict, string s1) const
{
	//LB_c: s2���洦������еõ����Ѵ���Ĵ�
	string s2="";
	unsigned int i,len;

	//LB_c: ѭ����ȡs1�е�ĳһ���ֽڽ��д���ֱ��s1Ϊ��
	while (!s1.empty()) {

		//LB_c: ȡs1�ĵ�һ���ֽڣ�ע������ch�������޶���Ϊunsigned����Ϊ���ֵ�ÿ���ֽڵ�ֵ����128
		unsigned char ch=(unsigned char) s1[0];

		//LB_c: ch<128˵��ch��һ��ASCII�ַ����ⲿ�ֹ���ASCII�ַ�
		if(ch<128) { // deal with ASCII
			i=1;
			len = s1.size();
			
			//LB_c: s1[i]Ϊ���ǻ��У�LF���ͻس���CR����ASCII�ַ���i++��Ҳ����i��¼�������ķ�LF��CR��ASCII�ַ��ĸ�����
			while (i<len && ((unsigned char)s1[i]<128) 
				&& (s1[i]!=10) && (s1[i]!=13)) { // LF, CR
				i++;
			}

			//LB_c: ���ch����LF��CR��SP���ո񣩣����ڵ�i���ֽڴ������ԷָSEPARATOR(Դ���ж���Ϊ"/  ")������Ĵ���Ӧ
			// ����������ģ��ں����ҽ����з�����
			if ((ch!=32) && (ch!=10) && (ch!=13)) {// SP, LF, CR
				s2 += s1.substr(0, i) + SEPARATOR;
			} else {
				//LB_c: ���chΪLF��CR����s1[0]��ʼ��i���ֽ����ݿ������Ѵ���s2�������ﲻ����ָ�������з֣�����Ҳ
				// �����Դ������⣬���ǵ�ch=SPʱʲôҲ��������ô����ַ���������Ҳ�ں�����з�����
				if (ch==10 || ch==13 || ch==32){
					s2+=s1.substr(0, i);
				}
			}

			//LB_c: ���s1û�д����꣬��s1��ֵΪʣ����ַ���
			if (i <= s1.size())	// added by yhf
				s1=s1.substr(i);
			else 
				break;		// yhf

			continue;

		//LB_c: else��chΪ��ASCII�ַ���������GBK�ַ���
		//LB_c: ������ȫ����д��else if (ch < 176)���ѽ⣡
		} else { 
			
			//LB_c: ch<176˵�������ı����������ķ��ţ�GBK�����Ǵ�176���Ͽ�ʼ����ģ�Ҳ�������е����ֽڶ��Ǵ���176����
			// �ҿ��Բ�GBK�ı���������ⲿ���Ǵ������ı�������������ŵġ�
			if (ch<176) { 
				i = 0;
				len = s1.length();

				//LB_c: GBK�����ֽ�ֵ���ڵ���161��С��176�������ı����ź�����������š��������whileѭ�����ǹ��˵���
				// Щ������ţ���������������ı�������ֹͣ��Ҳ����������Ų���Ҫ�з֣������ı�������Ҫ�з֡�while
				// �����е�ȡֵ������Щ�����ŵ�GBK����ֵ�����Բ�ѯGBK�������ա�
				while(i<len && ((unsigned char)s1[i]<176) && ((unsigned char)s1[i]>=161)
					//LB_c: ch��Ϊ���ı����ţ��� ��  �� �� �� ��
					&& (!((unsigned char)s1[i]==161 && ((unsigned char)s1[i+1]>=162 && (unsigned char)s1[i+1]<=168)))
					//LB_c: ch��Ϊ���ı����ţ��� �� �� �� ���� �� �� �� �� �� �� �� �� �� �� ��
					&& (!((unsigned char)s1[i]==161 && ((unsigned char)s1[i+1]>=171 && (unsigned char)s1[i+1]<=191)))
					//LB_c: ch��Ϊ���ı����ţ������� ���� ���� 
					&& (!((unsigned char)s1[i]==163 && ((unsigned char)s1[i+1]==172 || (unsigned char)s1[i+1]==161) 
													|| (unsigned char)s1[i+1]==168 || (unsigned char)s1[i+1]==169 
													|| (unsigned char)s1[i+1]==186 || (unsigned char)s1[i+1]==187 
													|| (unsigned char)s1[i+1]==191))) 
				{ 
					i=i+2; // �ٶ�û�а������
				}

				if (i==0) i=i+2;

				//LB_c: ���ch�������Ŀո�����s1�ĵ�i���ֽڴ�����ָ���з֣���������Ŀո����з֡�������ȻҲ�Ǵ���
				// ����ģ�������з�����
				if (!(ch==161 && (unsigned char)s1[1]==161)) { 
					if (i <= s1.size())	// yhf
						// �����ķǺ���˫�ֽ��ַ������������
						s2 += s1.substr(0, i) + SEPARATOR; 
					else break; // yhf
				}

				//LB_c: ���s1û�д����꣬��s1��ֵΪʣ����ַ���
				if (i <= s1.size())	// yhf
					s1=s1.substr(i);
				else break;		//yhf

				continue;
			}
		}
    
		//LB_c: ch��ȡֵ���������������Ǻ����ˣ���ch>=176Ϊ���ֱ������´����ִ�

		i = 2;
		len = s1.length();

		//LB_c: ���������ĺ��ִ���whileѭ��ֱ������ĳ�����ֽڲ��Ǻ���ֹͣ
		while(i<len && (unsigned char)s1[i]>=176) 
			i+=2;

		//LB_c: ��һ���ҵ���һ�������ĺ��ִ�s1(0,i)������������ķִʺ���SegmentHzStrMM�������ķִʡ�
		s2+=SegmentHzStrMM(dict, s1.substr(0,i));

		//LB_c: ���s1û�д����꣬��s1��ֵΪʣ����ַ���
		if (i <= len)	// yhf
			s1=s1.substr(i);
		else break;	// yhf
	}

	//LB_c: ��������Ժ�s2�д洢�ı��Ƿָ��Ľ�����м��Էָ����"/  "��������
	return s2;
}

// translate the encoded URL(%xx) to actual chars
void CHzSeg::Translate(char* SourceStr) const
{
	int i=0;
	int j=0;
	char *tempstr,tempchar1,tempchar2;

	tempstr = (char*)malloc(strlen(SourceStr) + 1);
	if(tempstr == NULL){
		return;
	}

	while (SourceStr[j])
	{
		if ((tempstr[i]=SourceStr[j])=='%'){
			if (SourceStr[j+1]>='A')
				tempchar1=((SourceStr[j+1]&0xdf)-'A')+10;
			else
				tempchar1=(SourceStr[j+1]-'0');
			if (SourceStr[j+2]>='A')
				tempchar2=((SourceStr[j+2]&0xdf)-'A')+10;
			else
				tempchar2=(SourceStr[j+2]-'0');
				tempstr[i]=tempchar1*16+tempchar2;
			j=j+2;
		}
		i++;
		j++;
	}
	tempstr[i]='\0';
	strcpy(SourceStr,tempstr);

	if(tempstr) free(tempstr);
}

/*
 * segment the image URL by '/'
 * omit the domain name
 */
string CHzSeg::SegmentURL(CDict &dict, string url) const
{
	string::size_type idx, nidx;
	char *curl = (char *)url.c_str();
	this->Translate(curl);
	url = curl;
	if((idx = url.find("http://", 0)) != string::npos)
	{
		if((nidx = url.find("/", 7)) != string::npos)
		{
			url = url.substr(nidx + 1);	// cut the part of sitename
		}
	}
	idx = 0;
	while((idx = url.find("/", idx)) != string::npos)
	{
		url.replace(idx, 1, SEPARATOR);	// replace "/" with SEPARATOR "/  "
		idx += 3;
	}
	if((idx = url.rfind(".")) != string::npos)
	{
		url = url.erase(idx);	// erase the file extension
	}

	url += "/  ";
	
	// segment the string whose length is greater than 8 (4 HZ_chars)
	idx = 0; nidx = 0;
	bool isover = false;
	string stmp;
	while(!isover)
	{
		if((nidx = url.find(SEPARATOR, idx)) == string::npos)
			isover = true;
		if(nidx - idx > 0)
		{
			stmp = url.substr(idx, nidx-idx);
			stmp = SegmentSentenceMM(dict, stmp);
			if ( stmp.size() >= 3)
				stmp.erase(stmp.length() - 3);	// erase the tail "/  "
			url = url.replace(idx, nidx-idx, stmp);
			idx += stmp.length() + 3;
		}
		else if(nidx == string::npos && idx < url.length())
		{
			stmp = url.substr(idx);
			stmp = SegmentSentenceMM(dict, stmp);
			stmp.erase(stmp.length() - 3);
			url = url.substr(0, idx) + stmp;
		}
		else
			idx = nidx + 3;
	}
	
	return url;
	
}


