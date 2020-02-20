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

  //相关操作符的重载
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

//压缩文件的类
class FileCompressHuffman
{
public:
	FileCompressHuffman();

  //压缩文件
	void CompressFile(const std::string& path);

  //解压缩
	void UnCompressFile(const std::string& path);

private:

  //获得哈弗曼编码
	void GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot);  //生成huffman编码

  //获取压缩后文件的头部信息
	void WriteHead(FILE* fOut, const std::string& filePostFix);

  //获取文件的后缀
	std::string GetFilePostFix(const std::string& fileName);

  //读取一行
	void ReadLine(FILE* fIn, std::string& strInfo);

private:
	std::vector<CharInfo> _fileInfo;
};
