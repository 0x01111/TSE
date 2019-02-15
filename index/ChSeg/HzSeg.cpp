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

//LB_c: 正向最大匹配法的中文分词实现，dict是分词所查询的词典对象，s1是中文字符串。
//LB_c: using Max-matching-method to segment the string s1 with dictory dict.
// Using Max Matching method to segment a character string.
string CHzSeg::SegmentHzStrMM (CDict &dict, string s1) const
{
	string s2="";				// store segment result
	while (!s1.empty()) { 
		unsigned int len=s1.size();
		//LB_c: MAX_WORD_LENGHT为设置的最大词长，TSE中设置为8个字节，即4个汉字
		if (len>MAX_WORD_LENGTH) len=MAX_WORD_LENGTH;

		//LB_c: 从s1头部取最大词长的子串作为待匹配词（如果是逆序最大匹配法则从s1的尾部取）
		string w=s1.substr(0, len);// the candidate word

		//LB_c: 在词典中查询w是否存在，CDict::IsWord函数判断字符串是否在词典中。在一个庞大的词典中查找词的效率是很关键
		// 的，这是影响该系统效率的一个因素之一，第5节中介绍过，TSE中的词典是用STL的map结构存储的，也就是红黑树的数据结
		// 构存储，而有的分词系统也用hash表存储，都是为了提高查询效率。
		bool isw=dict.IsWord(w);

		//LB_c: 如果在词典中没有找到则去掉最后一个字继续查询，直到w为一个单字为止。
		while (len>2 && isw==false) {	// if not a word
			len-=2;		// cut a word
			w=w.substr(0, len);
			isw=dict.IsWord(w);
		}

		//LB_c: 在匹配词w后加入分割符SEPARATOR进行分割。
		s2 += w + SEPARATOR;

		s1 = s1.substr(w.size());
	}

	return s2;
}


//LB_c: 中文分词预处理函数，过滤英文字符（ASCII字符）、特殊字符、中文标点符号等，将预处理后得到的汉字字符串交给
// SegmentHzStrMM函数进行中文分词。参数dict为词典对象，s1为待处理的字符串。
// process a sentence before segmentation
string CHzSeg::SegmentSentenceMM (CDict &dict, string s1) const
{
	//LB_c: s2保存处理过程中得到的已处理的串
	string s2="";
	unsigned int i,len;

	//LB_c: 循环读取s1中的某一个字节进行处理，直到s1为空
	while (!s1.empty()) {

		//LB_c: 取s1的第一个字节，注意这里ch的类型限定符为unsigned，因为汉字的每个字节的值大于128
		unsigned char ch=(unsigned char) s1[0];

		//LB_c: ch<128说明ch是一个ASCII字符，这部分过滤ASCII字符
		if(ch<128) { // deal with ASCII
			i=1;
			len = s1.size();
			
			//LB_c: s1[i]为不是换行（LF）和回车（CR）的ASCII字符则i++，也就是i记录了连续的非LF和CR的ASCII字符的个数。
			while (i<len && ((unsigned char)s1[i]<128) 
				&& (s1[i]!=10) && (s1[i]!=13)) { // LF, CR
				i++;
			}

			//LB_c: 如果ch不是LF、CR和SP（空格），则在第i个字节处插入以分割副SEPARATOR(源码中定义为"/  ")，这里的处理应
			// 该是有问题的，在后面我将进行分析。
			if ((ch!=32) && (ch!=10) && (ch!=13)) {// SP, LF, CR
				s2 += s1.substr(0, i) + SEPARATOR;
			} else {
				//LB_c: 如果ch为LF或CR，则将s1[0]开始的i个字节数据拷贝倒已处理串s2，即这里不插入分割符进行切分，这里也
				// 很明显存在问题，就是当ch=SP时什么也不处理，那么这段字符将丢弃，也在后面进行分析。
				if (ch==10 || ch==13 || ch==32){
					s2+=s1.substr(0, i);
				}
			}

			//LB_c: 如果s1没有处理完，则将s1赋值为剩余的字符串
			if (i <= s1.size())	// added by yhf
				s1=s1.substr(i);
			else 
				break;		// yhf

			continue;

		//LB_c: else中ch为非ASCII字符，即中文GBK字符。
		//LB_c: 这里完全可以写成else if (ch < 176)，费解！
		} else { 
			
			//LB_c: ch<176说明是中文标点或其他中文符号，GBK汉字是从176往上开始编码的，也就是所有的首字节都是大于176。大
			// 家可以查GBK的编码表。所以这部分是处理中文标点和中文特殊符号的。
			if (ch<176) { 
				i = 0;
				len = s1.length();

				//LB_c: GBK中首字节值大于等于161且小于176，是中文标点符号和其他特殊符号。下面这个while循环就是过滤掉这
				// 些特殊符号，如果遇到的是中文标点符号则停止，也就是特殊符号不需要切分，而中文标点符号需要切分。while
				// 条件中的取值就是这些标点符号的GBK编码值，可以查询GBK编码表对照。
				while(i<len && ((unsigned char)s1[i]<176) && ((unsigned char)s1[i]>=161)
					//LB_c: ch不为中文标点符号：、 。  ˉ ˇ ¨ 〃
					&& (!((unsigned char)s1[i]==161 && ((unsigned char)s1[i+1]>=162 && (unsigned char)s1[i+1]<=168)))
					//LB_c: ch不为中文标点符号：～ ‖ … ‘ ’“ ” 〔 〕 〈 〉 《 》 「 」 『 〗
					&& (!((unsigned char)s1[i]==161 && ((unsigned char)s1[i+1]>=171 && (unsigned char)s1[i+1]<=191)))
					//LB_c: ch不为中文标点符号：，！（ ）： ；？ 
					&& (!((unsigned char)s1[i]==163 && ((unsigned char)s1[i+1]==172 || (unsigned char)s1[i+1]==161) 
													|| (unsigned char)s1[i+1]==168 || (unsigned char)s1[i+1]==169 
													|| (unsigned char)s1[i+1]==186 || (unsigned char)s1[i+1]==187 
													|| (unsigned char)s1[i+1]==191))) 
				{ 
					i=i+2; // 假定没有半个汉字
				}

				if (i==0) i=i+2;

				//LB_c: 如果ch不是中文空格，则在s1的第i个字节处插入分割符切分，如果是中文空格则部切分。这里显然也是存在
				// 问题的，后面进行分析。
				if (!(ch==161 && (unsigned char)s1[1]==161)) { 
					if (i <= s1.size())	// yhf
						// 其他的非汉字双字节字符可能连续输出
						s2 += s1.substr(0, i) + SEPARATOR; 
					else break; // yhf
				}

				//LB_c: 如果s1没有处理完，则将s1赋值为剩余的字符串
				if (i <= s1.size())	// yhf
					s1=s1.substr(i);
				else break;		//yhf

				continue;
			}
		}
    
		//LB_c: ch的取值除了上面的情况就是汉字了，即ch>=176为汉字编码以下处理汉字串

		i = 2;
		len = s1.length();

		//LB_c: 查找连续的汉字串，while循环直到遇到某两个字节不是汉字停止
		while(i<len && (unsigned char)s1[i]>=176) 
			i+=2;

		//LB_c: 上一步找到了一个连续的汉字串s1(0,i)，这里调用中文分词函数SegmentHzStrMM进行中文分词。
		s2+=SegmentHzStrMM(dict, s1.substr(0,i));

		//LB_c: 如果s1没有处理完，则将s1赋值为剩余的字符串
		if (i <= len)	// yhf
			s1=s1.substr(i);
		else break;	// yhf
	}

	//LB_c: 处理结束以后，s2中存储的便是分割后的结果，中间以分割符（"/  "）隔开。
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


