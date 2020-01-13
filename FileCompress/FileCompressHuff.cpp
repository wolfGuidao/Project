#if 0
#define _CRT_SECURE_NO_WARNINGS 1
#include "HuffManTree.hpp"
#include "FileCompressHuff.h"
#include <assert.h>
#include <string>


FileCompressHuff::FileCompressHuff()
{
	_fileInfo.resize(256);

	for (int i = 0; i < 256; i++)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}

//获取文件后缀
string FileCompressHuff::GetFilePostFix(const string& filename)
{
	return filename.substr(filename.rfind('.'));
}


//每次读取文件一行
void FileCompressHuff::ReadLine(FILE* fIn, string& strInfo)
{
	assert(fIn);

	//feof判断文件指针是否在文件末尾
	while (!feof(fIn))
	{
		char ch = fgetc(fIn);
		if (ch == '\n')
		{
			break;
		}
		strInfo += ch;
	}
}

//往压缩文件内写解压时的基本信息
void FileCompressHuff::WriteHead(FILE* fOut, const string& filePostFix)
{
	assert(fOut);

	//写文件的后缀
	string strHead;
	strHead += GetFilePostFix(filePostFix);
	strHead += '\n';
	fwrite(filePostFix.c_str(), 1, filePostFix.size(), fOut);

	//写字符编码的行数
	size_t linecount = 0;
	string strChCount;
	char szValue[32] = { 0 };
	for (int i = 0; i < 256; i++)
	{
		CharInfo& charinfo = _fileInfo[i];
		if (charinfo._count != 0)
		{
			linecount++;
			strChCount += charinfo._ch;
			strChCount += ':';
			_itoa(charinfo._count, szValue, 10);
			strChCount += szValue;
			strChCount += '\n';
		}
	}
	_itoa(linecount, szValue, 10);
	strHead += szValue;
	strHead += '\n';

	strHead += strChCount;
	//strHead += '\n';

	fwrite(strHead.c_str(), 1, strHead.size(), fOut);

}



//压缩
void FileCompressHuff::CompressFile(const string& path)
{
	//1.统计源文件中每个字符出现的次数
	FILE* fIn = fopen(path.c_str(), "rb");
	if (fIn == nullptr)
	{
		assert(false);
		return;
	}

	unsigned char* pReadBuff = new unsigned char[1024];

	int rdSize = 0;
	while (1)
	{
		rdSize = fread(pReadBuff, 1, 1024, fIn);
		if (rdSize == 0)
		{
			//读取文件结束
			break;
		}

		for (int i = 0; i < rdSize; i++)
		{
			_fileInfo[pReadBuff[i]]._count++;
		}
	}

	//2.以每个字符出现的次数为权值，创建HuffMan树
	HuffManTree<CharInfo> t(_fileInfo,CharInfo());

	//3.获取每个字符的HuffMan编码
	GenerateHuffManCode(t.GetRoot());


	//4.用获取到的字符编码改写源文件
	FILE* fOut = fopen("2.txt", "wb");
	if (fOut == nullptr)
	{
		assert(false);
		return;
	}

	WriteHead(fOut,path);

	char ch = '0';//用来存改写后的数据
	int bitcount = 0;//每个ch只能放8位，
	fseek(fIn, 0, SEEK_SET);//注意前面已经读过文件一次，文件指针已经走到末尾，需要把文件指针指向文件的开始位置

	while (1)
	{
		rdSize = fread(pReadBuff, 1, 1024, fIn);
		if (rdSize == 0)
		{
			break;
		}
		//根据编码对读取到的内容进行从写
		for (int i = 0; i < rdSize; i++)//一个一个bite位放
		{
			string strCode = _fileInfo[pReadBuff[i]]._strCode;
			for (size_t j = 0; j < strCode.size(); j++)
			{
				ch <<= 1;
				if (strCode[j] == '1')
				{
					ch |= 1;
				}

				bitcount++;
				//一个ch只有8个bit位
				if (bitcount == 8)
				{
					bitcount = 0;
					fputc(ch, fOut);//写入文件
					ch = 0;
				}
			}
		}
	}
	//最后一次ch中可能不够8个bit位
	if (bitcount < 8)
	{
		ch <<= (8 - bitcount);
		fputc(ch, fOut);
	}

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}


//解压缩
void FileCompressHuff::UnCompressFile(const string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (fIn == nullptr)
	{
		assert(false);
		return;
	}

	//读取文件后缀
	string strFilePostFix;
	ReadLine(fIn, strFilePostFix);

	//读取字符信息的总行数
	string strCount;
	ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str());


	//读取字符信息
	for (int i = 0; i < lineCount; i++)
	{
		string strchCount;
		ReadLine(fIn, strchCount);

		//如果strchCount为空，说明读到了换行，应该把\n放进去，
		if (strchCount.empty())
		{
			strchCount += '\n';
			ReadLine(fIn, strchCount);
		}

		//A:3
		_fileInfo[(unsigned char)strchCount[0]]._count = atoi(strCount.c_str() + 2);//跳过字符本身和冒号拿到字符出现的次数
	}



	//还原HuffMan树
	HuffManTree<CharInfo> t;
	t.CreatHuffManTree(_fileInfo, CharInfo(0));


	//读取压缩数据，解压缩

	FILE* fOut = fopen("3.txt", "wb");
	if (fOut == nullptr)
	{
		assert(false);
		return;
	}

	unsigned char* pReadBuff = new unsigned char[1024];
	char ch = 0;
	HuffManTreeNode<CharInfo>* pCur = t.GetRoot();//获取到根节点
	size_t fileSize = pCur->_weight._count;//记录总字符的个数
	int uncount = 0;//记录已经获取到字符的个数
	while (1)
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);
		if (rdSize == 0)
		{
			break;
		}
		for (size_t i = 0; i < rdSize; i++)
		{
			//只需要将一个字节中的8个bite单独处理即可
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; pos++)
			{
				//规定0往左走，1往右走
				if (ch & 0x80)//高位位1
				{
					pCur = pCur->_pRight;
				}
				else//高位位0
				{
					pCur = pCur->_pLeft;
				}
				
				ch <<= 1;//每次都要左移

				if (pCur->_pLeft == nullptr && pCur->_pRight == nullptr)//走到叶子节点了
				{
					uncount++;
					fputc(pCur->_weight._ch, fOut);
					//如果已经获取够了，就直接break掉
					if (uncount == fileSize)
					{
						break;
					}
					
					pCur = t.GetRoot();
					
				}
			}
		}
	}
	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}


//获取HuffMan编码,从叶子节点往跟节点获取
void FileCompressHuff::GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot)
{
	if (pRoot == nullptr)
	{
		return;
	}
	GenerateHuffManCode(pRoot->_pLeft);
	GenerateHuffManCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{
		string strCode;
		HuffManTreeNode<CharInfo>* pCur = pRoot;
		HuffManTreeNode<CharInfo>* pParent = pCur->_pParent;

		while (pParent)
		{
			if (pParent->_pLeft == pCur)
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
		reverse(strCode.begin(), strCode.end());
		_fileInfo[pRoot->_weight._ch]._strCode = strCode;
	}
}

#endif

#define _CRT_SECURE_NO_WARNINGS 1 
#include "FileCompressHuff.h"
#include <algorithm>

FileCompressHuffman::FileCompressHuffman()
{
	_fileInfo.resize(256);
	for (int i = 0; i < 256; i++)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}

void FileCompressHuffman::CompressFile(const std::string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}
	//1、统计源文件中每个字符出现的次数
	unsigned char* pReadBuff = new unsigned char[1024];
	size_t readSize = 0;
	while (true)
	{
		readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)
		{
			break;
		}
		for (size_t i = 0; i < readSize; i++)
		{
			_fileInfo[pReadBuff[i]]._count++;
			//
		}
	}

	//2、以字符出现的次数为权值创建huffman树
	HuffmanTree<CharInfo> tree(_fileInfo, CharInfo());  //出现0次的无效的字符将不会参与huffman树的构造

	//3、获取每个字符的编码
	GenerateHuffmanCode(tree.GetRoot());

	//4、用获取到的编码重新改写源文件
	FILE* fOut = fopen("2.txt", "wb");
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}
	WriteHead(fOut, path);           //1111111111

	fseek(fIn, 0, SEEK_SET);
	int ch = 0;
	int bitCount = 0;
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
				ch <<= 1;
				if ('1' == strCode[j])
				{
					ch |= 1;
				}
				bitCount++;
				if (8 == bitCount)
				{
					fputc(ch, fOut);  //往文件中一次写入一个字节
					bitCount = 0;
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

void FileCompressHuffman::UnCompressFile(const std::string& path) {
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn) {
		//assert(false);
    perror("open file is error");
		return;
	}
	std::string strFilePostFix;
	ReadLine(fIn, strFilePostFix);

	std::string strCount;
	ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str());
	for (int i = 0; i < lineCount; ++i) {
		std::string strchCount;
		ReadLine(fIn, strchCount);
		if (strchCount.empty()) {
			strchCount += '\n';
			ReadLine(fIn, strchCount);
		}
		_fileInfo[(unsigned char)strchCount[0]]._count = atoi(strchCount.c_str() + 2);
	}
	HuffmanTree<CharInfo> t;
	t.CreateHuffmanTree(_fileInfo, CharInfo(0));

	std::string newFileName = "3" + strFilePostFix;
	FILE* fOut = fopen(newFileName.c_str(), "wb");
	char *pReadBuff = new char[1024];
	char ch = 0;
	HuffmanTreeNode<CharInfo>* pCur = t.GetRoot();
	size_t fileSize = pCur->_Weight._count;
	size_t unCount = 0;
	while (1) {
		size_t rdsize = fread(pReadBuff, 1, 1024, fIn);
		if (rdsize == 0) {
			break;
		}
		for (size_t i = 0; i < rdsize; ++i) {
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; ++pos) {
				if (ch & 0x80) {
					pCur = pCur->_pRight;
				}
				else {
					pCur = pCur->_pLeft;
				}
				ch <<= 1;
				if (nullptr == pCur->_pLeft&&nullptr == pCur->_pRight) {
					++unCount;
					fputc(pCur->_Weight._ch, fOut);
					pCur = t.GetRoot();
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
void FileCompressHuffman::ReadLine(FILE* fIn, std::string& strInfo) {
	assert(fIn);
	while (!feof(fIn))
	{
		char ch = fgetc(fIn);
		if (ch == '\n')
			break;
		strInfo += ch;
	}
}
void FileCompressHuffman::WriteHead(FILE* fOut, const std::string& fileName) {
	assert(fOut);
	std::string strHead;
	strHead += GetFilePostFix(fileName);
	strHead += '\n';

	size_t lineCount = 0;
	std::string strChCount;
	char szValue[32] = { 0 };
	for (int i = 0; i < 256; ++i) {
		CharInfo& charInfo = _fileInfo[i];
		if (charInfo._count) {
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

std::string  FileCompressHuffman::GetFilePostFix(const std::string& fileName) {
	return fileName.substr(fileName.rfind('.'));
}

void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot)  //生成huffman编码
{
	if (nullptr == pRoot)
	{
		return;
	}
	GenerateHuffmanCode(pRoot->_pLeft);
	GenerateHuffmanCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{//叶子节点，要编码的字符
		std::string strCode;
		HuffmanTreeNode<CharInfo>* pCur = pRoot;
		HuffmanTreeNode<CharInfo>* pParent = pCur->_pParent;

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
		reverse(strCode.begin(), strCode.end());

		_fileInfo[pRoot->_Weight._ch]._strCode = strCode;
	}
}
