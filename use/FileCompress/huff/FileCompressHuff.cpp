#define _CRT_SECURE_NO_WARNINGS 1 
#include "FileCompressHuff.h"
#include <algorithm>

//文件压缩构造函数
FileCompressHuffman::FileCompressHuffman()
{
  //给自己的成员初始化
	_fileInfo.resize(256);
	for (int i = 0; i < 256; i++)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}

//压缩文件，指明文件名
void FileCompressHuffman::CompressFile(const std::string& path)
{
  //注意这里的打开方式为 "b"代表以二进制的方式打开
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		//assert(false);
    perror("open file is error!\n");
		return;
	}

	//1、统计源文件中每个字符出现的次数,方便作为构建哈夫曼树的权值
  //注意这里用的是unsigned char 类型，是因为如果这里不是unsigned char类型，那么在压缩文件中有汉字
  //的时候就会崩溃
	unsigned char* pReadBuff = new unsigned char[1024];
	size_t readSize = 0;//记录每次读到的字节数
	while (true)
	{
		readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)//如果读到的字节数为0，说明文件指针已经走到文件的末尾，可以结束读取文件的操作了
		{
			break;
		}

    //每次对从文件中读取到的内容进行记录，保存相应字符出现的次数
		for (size_t i = 0; i < readSize; i++)
		{
			_fileInfo[pReadBuff[i]]._count++;
		}
	}

	//2、以字符出现的次数为权值创建huffman树
	HuffmanTree<CharInfo> Tree(_fileInfo, CharInfo());  //出现0次的无效的字符将不会参与huffman树的构造

	//3、获取每个字符的编码
	GenerateHuffmanCode(Tree.GetRoot());

	//4、用获取到的编码重新改写源文件
	FILE* fOut = fopen("2.txt", "wb");
	if (nullptr == fOut)
	{
		//assert(false);
    perror("open file is error\n");
		return;
	}

  //写入头部信息，方便解压缩
	WriteHead(fOut, path);           //1111111111

  //注意，fIn文件指针在上面的读取结束后，已经走到文件的结尾，所以这里要把文件指针移到开始的位置
	fseek(fIn, 0, SEEK_SET);

	char ch = 0;//每次往ch中写
	int bitCount = 0;//记录已经读取到的字节数
	while (true)
	{
		readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)
		{
			break;
		}
		//根据字节的编码对读取到的内容进行重写
		for (size_t i = 0; i < readSize; i++)
		{
			std::string strCode = _fileInfo[pReadBuff[i]]._strCode;
			for (size_t j = 0; j < strCode.size(); j++)
			{
        //如果是0，进不去if就直接左移，如果是1就把相应位置换为1
				ch <<= 1;
				if ('1' == strCode[j])
				{
					ch |= 1;
				}

				bitCount++;

        //因为ch只能放8个字节，所以每次都要进行判断
				if (8 == bitCount)
				{
					fputc(ch, fOut);  //往文件中一次写入一个字节
					bitCount = 0;//清空
					ch = 0;
				}
			}
		}
	}

	//最后一次ch中可能不够8个bit位
	if (bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fOut);
	}

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//解压缩
void FileCompressHuffman::UnCompressFile(const std::string& path) {
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn) {
		//assert(false);
    perror("open file is error");
		return;
	}

  //文件后缀
	std::string strFilePostFix;
	ReadLine(fIn, strFilePostFix);

  //读取文件数据的行数
	std::string strCount;
	ReadLine(fIn, strCount);

	int lineCount = atoi(strCount.c_str());
	for (int i = 0; i < lineCount; ++i) 
  {
    //记录读取每一行的数据
		std::string strchCount;
		ReadLine(fIn, strchCount);

    //如果读到的数据为空，说明可能读到了换行符
		if (strchCount.empty()) 
    {
			strchCount += '\n';
			ReadLine(fIn, strchCount);
		}

    //因为数据的格式是 A:3，记录读取到的相关字符信息方便构建哈夫曼树,
		_fileInfo[(unsigned char)strchCount[0]]._count = atoi(strchCount.c_str() + 2);
	}

	HuffmanTree<CharInfo> t;
	t.CreateHuffmanTree(_fileInfo, CharInfo(0));//构建哈夫曼树

  //解压缩后的文件名
	std::string newFileName = "3" + strFilePostFix;

	FILE* fOut = fopen(newFileName.c_str(), "wb");
  if(fOut == nullptr)
  {
    perror("open file is error\n");
    return ;
  }

	char *pReadBuff = new char[1024];
	char ch = 0;
	HuffmanTreeNode<CharInfo>* pCur = t.GetRoot();//拿到根节点，也就拿到了整个哈夫曼树的权值之和
	size_t fileSize = pCur->_Weight._count;//记录待解压的总字节数
	size_t unCount = 0;//记录已经解压字符的个数
	while (1) 
  {
		size_t rdsize = fread(pReadBuff, 1, 1024, fIn);
		if (rdsize == 0) 
    {
			break;
		}
    
		for (size_t i = 0; i < rdsize; ++i) 
    {
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; ++pos) 
      {
        //0x80--->1000 0000 所以是用来判断一个字节的高位是1还是0的
        //规定 1--->往哈夫曼树的右子树走  0--->往哈夫曼树的左子树走
				if (ch & 0x80) 
        {
					pCur = pCur->_pRight;
				}
				else 
        {
					pCur = pCur->_pLeft;
				}

				ch <<= 1;

        //如果pCur左右孩子都为空，说明走到叶子节点了，就可以获取到具体的字符了
				if (nullptr == pCur->_pLeft&&nullptr == pCur->_pRight) 
        {
					++unCount;
					fputc(pCur->_Weight._ch, fOut);
					pCur = t.GetRoot();

          //因为每次不一定刚好是整字节数，所以防止多读，需要进行判断
					if (unCount == fileSize)
						break;

				}
			}
		}

	}
	fclose(fIn);
	fclose(fOut);
	delete[] pReadBuff;
}

//读一行函数
void FileCompressHuffman::ReadLine(FILE* fIn, std::string& strInfo) 
{
	assert(fIn);

  //注意这里的feof是针对二进制文件的，因为我们为了解决可以压缩汉字的相关问题，选择以二进制的方式打开，并不是以文本文件的方式打开，文本文件的结束标识是EOF，而二进制文件的结束标识是FEOF
	while (!feof(fIn))
	{
		char ch = fgetc(fIn);
		if (ch == '\n')
			break;
		strInfo += ch;
	}
}

//写文件的头部信息，方便解压缩
void FileCompressHuffman::WriteHead(FILE* fOut, const std::string& fileName) {
	assert(fOut);
	std::string strHead;

  //首先是文件的后缀
	strHead += GetFilePostFix(fileName);
	strHead += '\n';

  //其次是文件的数据行数
	size_t lineCount = 0;//记录数据的行数

	std::string strChCount;//记录每行数据的内容

	char szValue[32] = { 0 };//记录每个字符出现的次数
	for (int i = 0; i < 256; ++i) 
  {
		CharInfo& charInfo = _fileInfo[i];
		if (charInfo._count) //判断是否为有效字符
    {
			lineCount++;
			strChCount += _fileInfo[i]._ch;
			strChCount += ':';
			//_itoa(charInfo._count, szValue, 10);
      sprintf(szValue,"%lu",charInfo._count);

			strChCount += szValue;
			strChCount += '\n';
		}
	}
	//_itoa(lineCount, szValue, 10);
  sprintf(szValue,"%lu",lineCount);

	strHead += szValue;
	strHead += '\n';
	strHead += strChCount;

	fwrite(strHead.c_str(), 1, strHead.size(), fOut);
}

//获得文件的后缀
std::string  FileCompressHuffman::GetFilePostFix(const std::string& fileName) 
{
	return fileName.substr(fileName.rfind('.'));
}

//生成字符的编码----注意是从叶子节点往根节点获取的
void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot)  //生成huffman编码
{
	if (nullptr == pRoot)
	{
		return;
	}

	GenerateHuffmanCode(pRoot->_pLeft);
	GenerateHuffmanCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{
    //叶子节点，要编码的字符
		std::string strCode;
		HuffmanTreeNode<CharInfo>* pCur = pRoot;
		HuffmanTreeNode<CharInfo>* pParent = pCur->_pParent;

    //根据前面的规定，右子树为1； 左子树为0
		while (pParent)
		{
			if (pCur == pParent->_pLeft)
			{
				strCode += '0';
			}
			else
			{
				strCode += '1';
			}
			pCur = pParent;
			pParent = pCur->_pParent;
		}

    //因为是从叶子节点往根节点获取的，所以编码是反的，需要进行反转
		reverse(strCode.begin(), strCode.end());

		_fileInfo[pRoot->_Weight._ch]._strCode = strCode;
	}
}
