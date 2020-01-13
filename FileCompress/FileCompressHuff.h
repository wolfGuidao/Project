#if 0
#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct CharInfo
{
	unsigned char _ch;//字符
	size_t _count;//字符出现的次数
	string _strCode;//字符编码

	CharInfo(size_t count = 0)
		:_count(count)
	{

	}

	CharInfo operator+ (const CharInfo& c)const
	{
		return CharInfo(_count + c._count);
	}

	bool operator>(const CharInfo& c) const
	{
		return _count > c._count;
	}

	bool operator==(const CharInfo& c) const
	{
		return _count == c._count;
	}
};

class FileCompressHuff
{
public:
	FileCompressHuff();
	void CompressFile(const string& path);
	void UnCompressFile(const string& path);
	
	
private:
	void GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot);
	void WriteHead(FILE* fOut, const string& filePostFix);
	string GetFilePostFix(const string& filename);
	void ReadLine(FILE* fIn, string& strInfo);


private:
	vector<CharInfo> _fileInfo;
};

#endif

//基于huffman的压缩
#pragma once

#include <assert.h>
#include <vector>
#include <string>
#include "HuffManTree.hpp"


//将权值用结构体表示,所以需要在huffman中的操作符进行重载
struct CharInfo
{
	unsigned char _ch;  //具体字符
	size_t _count;  //字符出现次数
	std::string _strCode;  //字符编码

	CharInfo(size_t count = 0)
		: _count(count)
	{}

	CharInfo operator+(const CharInfo& ch) const
	{
		return CharInfo(_count + ch._count);
	}

	bool operator>(const CharInfo& ch) const
	{
		return _count > ch._count;
	}

	bool operator==(const CharInfo& ch) const
	{
		return _count == ch._count;
	}
};

class FileCompressHuffman
{
public:
	FileCompressHuffman();
	void CompressFile(const std::string& path);
	void UnCompressFile(const std::string& path);

private:
	void GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot);  //生成huffman编码
	void WriteHead(FILE* fOut, const std::string& filePostFix);
	std::string GetFilePostFix(const std::string& fileName);
	void ReadLine(FILE* fIn, std::string& strInfo);

private:
	std::vector<CharInfo> _fileInfo;
};
