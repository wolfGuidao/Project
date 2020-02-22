#pragma once
#include<string>
#include"HashTable.cpp"

class LZ77 
{
  public:

    LZ77();
    ~LZ77();
    void CompressionFile(const std::string& fileName);
    void UnCompressionFile(const std::string& fileName);

  private:

    //获取最长的匹配串
    USH  LongestMatch(USH matchHead, USH &curMatchDist, USH start);

    //写压缩文件时的标记信息
    void WriteFlag(FILE* file, UCH& chNum, UCH& bitCount, bool isLen);

  private:

    UCH* pWin_;
    HashTable ht_;

};
