#pragma once
#include<string>
#include"HashTable.h"

class LZ77 {
  public:
    LZ77();
    ~LZ77();
    void CompressFile(const std::string& strFilePath);
    void UNCompressFile(const std::string& strFilePath);
  private:
    USH LongestMatch(USH matchHead, USH& curMatchDist, USH start);      //找最长匹配
    void WriteFlag(FILE* fOUT, UCH& chFlag, UCH& bitCount, bool isLen); //写标记文件
    void MergeFile(FILE* fOut, ULL fileSize);
    void FillWindow(FILE* fIn, size_t& lookAhead, USH& start);
  private:
    UCH* pWin_;                                                         //用来保存待压缩数据的缓冲区
    HashTable ht_;

};
